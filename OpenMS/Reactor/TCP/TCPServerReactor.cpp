/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "TCPServerReactor.h"
#include "TCPChannel.h"
#include <asio.hpp>

TCPServerReactor::TCPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_tcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Backlog(backlog ? backlog : 128),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void TCPServerReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = MSThread([=, &promise, this]()
	{
		asio::io_context loop;
		using namespace asio::ip;

		do
		{
			// Bind and listen to the socket

			auto reactor = this;
			tcp::acceptor server(loop);
			tcp::endpoint endpoint(address::from_string(m_Address->getAddress()), m_Address->getPort());
			asio::error_code error;
			error = server.open(endpoint.protocol(), error);
			if (error)
			{
				MS_ERROR("failed to open: %s", error.message().c_str());
				break;
			}
			error = server.bind(endpoint, error);
			if (error)
			{
				MS_ERROR("failed to bind: %s", error.message().c_str());
				break;
			}
			error = server.listen((int32_t)m_Backlog, error);
			if (error)
			{
				MS_ERROR("failed to listen: %s", error.message().c_str());
				break;
			}

			// Get the actual ip and port number

			if (true)
			{
				MSRef<ISocketAddress> localAddress;
				auto address = server.local_endpoint().address().to_string();
				auto portNum = server.local_endpoint().port();
				auto family = server.local_endpoint().protocol().family();
				if (family == AF_INET) localAddress = MSNew<IPv4Address>(address, portNum);
				else if (family == AF_INET6) localAddress = MSNew<IPv6Address>(address, portNum);
				else MS_ERROR("unknown address family: %d", family);
				if (localAddress == nullptr) break;
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			// Read and write data in async way

			MSLambda<void()> accept_func;
			MSLambda<void(MSHnd<TCPChannel> channel)> read_func;
			MSLambda<void(MSHnd<TCPChannel> channel, MSRef<IChannelEvent> event)> write_func;

			read_func = [&](MSHnd<TCPChannel> channel)
			{
				if (auto client = channel.lock())
				{
					auto socket = client->getSocket();
					auto buffer = client->getBuffer();
					socket->async_read_some(asio::buffer(buffer.data(), buffer.size()), [=, &read_func](asio::error_code error, size_t length)
					{
						MS_INFO("tcp async read: %s 长度 %u 状态 %d", channel.lock()->getRemote().lock()->getString().c_str(), length, error.value());
						if (error)
						{
							MS_ERROR("can't read from socket: %s", error.message().c_str());
							reactor->onDisconnect(channel.lock());
						}
						else
						{
							auto event = MSNew<IChannelEvent>();
							event->Message = MSString(buffer.data(), length);
							event->Channel = channel;
							MS_INFO("read_func 0");
							reactor->onInbound(event);
							MS_INFO("read_func 1");
							read_func(channel);
							MS_INFO("read_func 2");
						}
					});
				}
			};

			write_func = [&](MSHnd<TCPChannel> channel, MSRef<IChannelEvent> event)
			{
				if (auto client = channel.lock())
				{
					auto socket = client->getSocket();
					socket->async_write_some(asio::buffer(event->Message), [=, &write_func](asio::error_code error, size_t length) mutable
					{
						MS_INFO("tcp async write: %s 长度 %u 状态 %d", channel.lock()->getRemote().lock()->getString().c_str(), length, error.value());
						if (error)
						{
							if (event->Promise) event->Promise->set_value(false);

							MS_ERROR("can't write to socket: %s", error.message().c_str());
							reactor->onDisconnect(channel.lock());
						}
						else
						{
							if (event->Promise) event->Promise->set_value(true);

							MSMutexLock lock(reactor->m_EventLock);
							if (reactor->m_EventQueue.empty())
							{
								reactor->m_Sending = false;
							}
							else
							{
								event = reactor->m_EventQueue.front();
								reactor->m_EventQueue.pop();
								write_func(MSCast<TCPChannel>(event->Channel.lock()), event);
							}
						}
					});
				}
			};

			accept_func = [&]()
			{
				server.async_accept([=, &accept_func, &read_func](asio::error_code error, tcp::socket client)
				{
					if (error) MS_ERROR("failed to accept: %s", error.message().c_str());
					else
					{
						// Get the actual ip and port number

						MSRef<ISocketAddress> localAddress, remoteAddress;
						{
							auto address = client.local_endpoint().address().to_string();
							auto portNum = client.local_endpoint().port();
							auto family = client.local_endpoint().protocol().family();
							if (family == AF_INET) localAddress = MSNew<IPv4Address>(address, portNum);
							else if (family == AF_INET6) localAddress = MSNew<IPv6Address>(address, portNum);
							else MS_ERROR("unknown address family: %d", family);
						}
						{
							auto address = client.remote_endpoint().address().to_string();
							auto portNum = client.remote_endpoint().port();
							auto family = client.remote_endpoint().protocol().family();
							if (family == AF_INET) remoteAddress = MSNew<IPv4Address>(address, portNum);
							else if (family == AF_INET6) remoteAddress = MSNew<IPv6Address>(address, portNum);
							else MS_ERROR("unknown address family: %d", family);
						}
						if (localAddress == nullptr || remoteAddress == nullptr)
						{
							client.close();
							return;
						}

						auto channel = MSNew<TCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), std::move(client));
						reactor->onConnect(channel);
						read_func(channel);
					}
					accept_func();
				});
			};

			m_FireAsync = [&]()
			{
				loop.post([=]()
				{
					if (reactor->m_Sending) return;
					reactor->m_Sending = true;
					MSRef<IChannelEvent> event;
					{
						MSMutexLock lock(reactor->m_EventLock);
						if (reactor->m_EventQueue.empty()) return;
						event = reactor->m_EventQueue.front();
						reactor->m_EventQueue.pop();
					}
					write_func(MSCast<TCPChannel>(event->Channel.lock()), event);
				});
			};

			accept_func();

			// Run the event loop

			asio::steady_timer timer(loop);
			MSLambda<void()> timer_func;
			timer_func = [&]()
			{
				timer.expires_after(std::chrono::milliseconds(1000));
				timer.async_wait([&](asio::error_code)
				{
					if (m_Running == false) loop.stop();
					else timer_func();
				});
			};
			timer_func();
			m_Connect = true;
			promise.set_value();
			loop.run();
			m_Connect = false;
			m_FireAsync = nullptr;

			// Close all channels

			{
				auto channels = m_ChannelMap;
				for (auto& channel : channels)
				{
					if (channel.second) onDisconnect(channel.second);
				}
				m_ChannelMap.clear();
			}

			MS_PRINT("closed server");
			return;

		} while (false);

		promise.set_value();
	});

	future.wait();
}

void TCPServerReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();

	m_ChannelMap.clear();
}

MSHnd<IChannelAddress> TCPServerReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void TCPServerReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false || event == nullptr) return;
	auto channel = event->Channel.lock();
	if (channel && channel->running()) channel->write(event);
}

void TCPServerReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelMap[hashName] = channel;
	ChannelReactor::onConnect(channel);
}

void TCPServerReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelMap.erase(hashName);
	ChannelReactor::onDisconnect(channel);
}

void TCPServerReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
	if (m_Sending == false) m_FireAsync();
}