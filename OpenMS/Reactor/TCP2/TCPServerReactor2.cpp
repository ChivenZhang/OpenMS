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
#include "TCPServerReactor2.h"
#include "TCPChannel2.h"
#include <asio.hpp>

TCPServerReactor2::TCPServerReactor2(MSRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_tcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Backlog(backlog ? backlog : 128),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void TCPServerReactor2::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = MSThread([=, &promise, this]()
	{
		asio::io_context loop;
		using namespace asio::ip;

		// Bind and listen to the socket

		do
		{
			auto reactor = this;
			tcp::acceptor server(loop, tcp::endpoint(address::from_string(m_Address->getAddress()), m_Address->getPort()));
			if (server.is_open() == false)
			{
				MS_ERROR("failed to listen and accept");
				break;
			}

			// Get the actual ip and port number

			if (true)
			{
				MSRef<ISocketAddress> localAddress;
				auto address = server.local_endpoint().address().to_string();
				auto portNum = server.local_endpoint().port();
				auto family = server.local_endpoint().protocol().family();
				if (family == AF_INET)
				{
					localAddress = MSNew<IPv4Address>(address, portNum);
				}
				else if (family == AF_INET6)
				{
					localAddress = MSNew<IPv6Address>(address, portNum);
				}
				else MS_ERROR("unknown address family: %d", family);
				if (localAddress == nullptr) break;
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			MSLambda<void()> accept_func;
			MSLambda<void(MSHnd<TCPChannel2> channel)> read_func;
			MSLambda<void(MSHnd<TCPChannel2> channel, MSRef<IChannelEvent> event)> write_func;
			accept_func = [&]()
			{
				server.async_accept([&](asio::error_code error, tcp::socket client)
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
							if (family == AF_INET)
							{
								localAddress = MSNew<IPv4Address>(address, portNum);
							}
							else if (family == AF_INET6)
							{
								localAddress = MSNew<IPv6Address>(address, portNum);
							}
							else MS_ERROR("unknown address family: %d", family);
						}
						{
							auto address = client.remote_endpoint().address().to_string();
							auto portNum = client.remote_endpoint().port();
							auto family = client.remote_endpoint().protocol().family();
							if (family == AF_INET)
							{
								remoteAddress = MSNew<IPv4Address>(address, portNum);
							}
							else if (family == AF_INET6)
							{
								remoteAddress = MSNew<IPv6Address>(address, portNum);
							}
							else MS_ERROR("unknown address family: %d", family);
						}
						if (localAddress == nullptr || remoteAddress == nullptr)
						{
							client.close();
							return;
						}

						auto channel = MSNew<TCPChannel2>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), std::move(client));
						reactor->onConnect(channel);

						// Start reading data from the client

						read_func(channel);
					}
					accept_func();
				});
			};

			read_func = [&](MSHnd<TCPChannel2> channel)
			{
				if (auto client = channel.lock())
				{
					auto socket = client->getHandle();
					auto buffer = client->getRBuffer();
					socket->async_read_some(asio::buffer(buffer.data(), buffer.size()), [=](asio::error_code error, size_t length)
					{
						if (error)
						{
							MS_ERROR("can't read from socket: %s", error.message().c_str());
							reactor->onDisconnect(client->shared_from_this());
						}
						else
						{
							auto event = MSNew<IChannelEvent>();
							event->Message = MSString(buffer.data(), length);
							event->Channel = channel;
							reactor->onInbound(event);

							read_func(channel);
						}
					});
				}
			};

			write_func = [&](MSHnd<TCPChannel2> channel, MSRef<IChannelEvent> event)
			{
				if (auto client = channel.lock())
				{
					auto socket = client->getHandle();

					socket->async_write_some(asio::buffer(event->Message), [&, event, channel](asio::error_code error, size_t length)
					{
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
								auto nextEvent = reactor->m_EventQueue.front();
								reactor->m_EventQueue.pop();
								write_func(channel, nextEvent);
							}
						}
					});
				}
			};

			accept_func();

			// Run the event loop

			m_EventAsync = [&]()
			{
				loop.post([&]()
				{
					if (reactor->m_Sending) return;
					reactor->m_Sending = true;
					MSMutexLock lock(reactor->m_EventLock);
					if (reactor->m_EventQueue.empty()) return;
					auto event = reactor->m_EventQueue.front();
					reactor->m_EventQueue.pop();
					write_func(MSCast<TCPChannel2>(event->Channel.lock()), event);
				});
			};
			asio::steady_timer timer(loop);
			MSLambda<void()> timer_func;
			timer_func = [&]()
			{
				timer.expires_after(std::chrono::milliseconds(1000));
				timer.async_wait([&](asio::error_code error)
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
			m_EventAsync = nullptr;

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

void TCPServerReactor2::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();

	m_ChannelMap.clear();
}

MSHnd<IChannelAddress> TCPServerReactor2::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void TCPServerReactor2::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false || event == nullptr) return;
	auto channel = event->Channel.lock();
	if (channel && channel->running()) channel->write(event);
}

void TCPServerReactor2::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelMap[hashName] = channel;
	ChannelReactor::onConnect(channel);
}

void TCPServerReactor2::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelMap.erase(hashName);
	ChannelReactor::onDisconnect(channel);
}

void TCPServerReactor2::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
	if (m_Sending == false) m_EventAsync();
}