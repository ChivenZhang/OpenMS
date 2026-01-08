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
#include "TCPClientReactor.h"
#include "TCPChannel.h"
#include <asio.hpp>

TCPClientReactor::TCPClientReactor(MSRef<ISocketAddress> address, size_t workerNum, callback_tcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void TCPClientReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();
	m_EventThread = MSThread([=, this, &promise]()
	{
		asio::io_context loop;
		using namespace asio::ip;

		do
		{
			// Bind and listen to the socket

			auto reactor = this;
			tcp::socket client(loop);
			tcp::endpoint endpoint(address::from_string(m_Address->getAddress()), m_Address->getPort());
			asio::error_code error;
			error = client.open(endpoint.protocol(), error);
			if (error)
			{
				MS_ERROR("failed to open: %s", error.message().c_str());
				break;
			}
			error = client.connect(endpoint, error);
			if (error)
			{
				MS_ERROR("failed to connect: %s", error.message().c_str());
				break;
			}

			// Get the actual ip and port number

			MSRef<ISocketAddress> localAddress, remoteAddress;
			if (true)
			{
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
					break;
				}
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			// Read and write data in async way

			MSLambda<void(MSHnd<TCPChannel> channel)> read_func;
			MSLambda<void(MSHnd<TCPChannel> channel, MSRef<IChannelEvent> event)> write_func;

			read_func = [=, &read_func](MSHnd<TCPChannel> channel)
			{
				if (auto _channel = channel.lock())
				{
					auto socket = _channel->getSocket();
					auto buffer = _channel->getBuffer();
					socket->async_read_some(asio::buffer(buffer.data(), buffer.size()), [=, &read_func](asio::error_code error, size_t length)
					{
						MS_DEBUG("tcp async read: %s 长度 %u 状态 %d", channel.lock()->getRemote().lock()->getString().c_str(), (uint32_t)length, error.value());
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
							reactor->onInbound(event);
							read_func(channel);
						}
					});
				}
			};

			write_func = [=, &write_func](MSHnd<TCPChannel> channel, MSRef<IChannelEvent> event)
			{
				if (auto _channel = channel.lock())
				{
					auto socket = _channel->getSocket();
					if (socket->is_open() == false) return;
					socket->async_write_some(asio::buffer(event->Message), [=, &write_func](asio::error_code error, size_t length) mutable
					{
						MS_DEBUG("tcp async write: %s 长度 %u 状态 %d", channel.lock()->getRemote().lock()->getString().c_str(), (uint32_t)length, error.value());
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

			m_FireAsync = [&]()
			{
				loop.post([&]()
				{
					if (reactor->m_Sending) return;
					reactor->m_Sending = true;
					MSMutexLock lock(reactor->m_EventLock);
					if (reactor->m_EventQueue.empty()) return;
					auto event = reactor->m_EventQueue.front();
					reactor->m_EventQueue.pop();
					write_func(MSCast<TCPChannel>(event->Channel.lock()), event);
				});
			};

			if (true)
			{
				auto channel = MSNew<TCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), std::move(client));
				reactor->onConnect(channel);
				read_func(MSCast<TCPChannel>(m_Channel));
			}

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

			// Close channel

			{
				if (m_Channel) onDisconnect(m_Channel);
				m_Channel = nullptr;
			}

			MS_PRINT("closed client");
			return;

		} while (false);

		promise.set_value();
	});

	future.wait();
}

void TCPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_Channel = nullptr;
}

MSHnd<IChannelAddress> TCPClientReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void TCPClientReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	if (channel && channel->running()) channel->write(event);
}

void TCPClientReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	m_Connect = true;
	m_Channel = channel;
	ChannelReactor::onConnect(channel);
}

void TCPClientReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_INFO("rejected from %s", channel->getRemote().lock()->getString().c_str());

	m_Connect = false;
	m_Channel = nullptr;
	ChannelReactor::onDisconnect(channel);
}

void TCPClientReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
	if (m_Sending == false) m_FireAsync();
}