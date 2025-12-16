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
	m_EventThread = MSThread([=, &promise]()
	{
		asio::io_context loop;
		using namespace asio::ip;

		// Bind and listen to the socket

		do
		{
			auto reactor = this;
			tcp::socket client(loop);
			MSLambda<void(MSHnd<TCPChannel> channel)> read_func;
			MSLambda<void(MSHnd<TCPChannel> channel, MSRef<IChannelEvent> event)> write_func;

			read_func = [&](MSHnd<TCPChannel> channel)
			{
				if (auto client = channel.lock())
				{
					auto socket = client->getSocket();
					auto buffer = client->getBuffer();
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

			write_func = [&](MSHnd<TCPChannel> channel, MSRef<IChannelEvent> event)
			{
				if (auto client = channel.lock())
				{
					auto socket = client->getSocket();

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

			asio::error_code error;
			client.connect(tcp::endpoint(address::from_string(m_Address->getAddress()), m_Address->getPort()), error);
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
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", localAddress->getAddress().c_str(), localAddress->getPort());

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
					write_func(MSCast<TCPChannel>(event->Channel.lock()), event);
				});
			};

			auto channel = MSNew<TCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), std::move(client));
			reactor->onConnect(channel);
			read_func(channel);

			// Run the event loop
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
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	m_Connect = false;
	m_Channel = nullptr;
	ChannelReactor::onDisconnect(channel);
}

void TCPClientReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
	if (m_Sending == false) m_EventAsync();
}