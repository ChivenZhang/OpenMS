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
#include "UDPClientReactor.h"
#include "UDPChannel.h"

UDPClientReactor::UDPClientReactor(MSRef<ISocketAddress> address, bool broadcast, bool multicast, size_t workerNum, callback_udp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Broadcast(broadcast),
	m_Multicast(multicast),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void UDPClientReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = MSThread([=, &promise]()
	{
		asio::io_context loop;
		using namespace asio::ip;

		do
		{
			// Bind and listen to the socket

			auto reactor = this;
			udp::socket client(loop);
			udp::endpoint endpoint(address::from_string(m_Address->getAddress()), m_Address->getPort());
			asio::error_code error;
			error = client.open(endpoint.protocol(), error);
			if (error)
			{
				MS_ERROR("failed to open: %s", error.message().c_str());
				break;
			}
			error = client.set_option(udp::socket::broadcast(m_Broadcast), error);
			if (error)
			{
				MS_ERROR("failed to set broadcast option: %s", error.message().c_str());
				break;
			}
			if (m_Multicast)
			{
				error = client.set_option(udp::socket::reuse_address(true), error);
				if (error)
				{
					MS_ERROR("failed to set reuse_address option: %s", error.message().c_str());
					break;
				}
				error = client.set_option(multicast::join_group(endpoint.address()), error);
				if (error)
				{
					MS_ERROR("failed to set multicast option: %s", error.message().c_str());
					break;
				}
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
					error = client.shutdown(tcp::socket::shutdown_both, error);
					if (error) MS_ERROR("failed to shutdown: %s", error.message().c_str());
					error = client.close(error);
					if (error) MS_ERROR("failed to close: %s", error.message().c_str());
					return;
				}
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			// Read and write data in async way

			MSLambda<void(MSHnd<UDPChannel> channel)> read_func;
			MSLambda<void(MSHnd<UDPChannel> channel, MSRef<IChannelEvent> event)> write_func;

			char buffer[2048];

			read_func = [&](MSHnd<UDPChannel> channel)
			{
				if (auto _channel = channel.lock())
				{
					client.async_receive(asio::buffer(buffer), [=, &read_func](asio::error_code error, size_t length)
					{
						if (error)
						{
							MS_ERROR("can't read from socket: %s", error.message().c_str());
							reactor->onDisconnect(channel.lock());
						}
						else
						{
							auto event = MSNew<IChannelEvent>();
							event->Message = MSString(buffer, length);
							event->Channel = channel;
							reactor->onInbound(event);
							read_func(channel);
						}
					});
				}
			};

			write_func = [&](MSHnd<UDPChannel> channel, MSRef<IChannelEvent> event)
			{
				if (auto _channel = channel.lock())
				{
					client.async_send(asio::buffer(event->Message), [=, &write_func](asio::error_code error, size_t length)
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
					write_func(MSCast<UDPChannel>(event->Channel.lock()), event);
				});
			};

			auto channel = MSNew<UDPChannel>(this, localAddress, remoteAddress, (uint32_t)(rand() % m_WorkerList.size()), client.remote_endpoint());
			onConnect(channel);
			read_func(MSCast<UDPChannel>(m_Channel));

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

			if (true)
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

void UDPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();

	m_Channel = nullptr;
}

MSHnd<IChannelAddress> UDPClientReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void UDPClientReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->write(event);
}

void UDPClientReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	m_Connect = true;
	m_Channel = channel;
	ChannelReactor::onConnect(channel);
}

void UDPClientReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	m_Connect = false;
	m_Channel = nullptr;
	ChannelReactor::onDisconnect(channel);
}

void UDPClientReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
	if (m_Sending == false) m_FireAsync();
}