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
#include "UDPServerReactor.h"
#include "UDPChannel.h"
#include <asio.hpp>

UDPServerReactor::UDPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, bool broadcast, bool multicast, size_t workerNum, callback_udp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Backlog(backlog ? backlog : 128),
	m_Broadcast(broadcast),
	m_Multicast(multicast),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void UDPServerReactor::startup()
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
			udp::socket server(loop);
			udp::endpoint endpoint(address::from_string(m_Address->getAddress()), m_Address->getPort());
			asio::error_code error;
			error = server.open(endpoint.protocol(), error);
			if (error)
			{
				MS_ERROR("failed to open: %s", error.message().c_str());
				break;
			}
			error = server.set_option(udp::socket::broadcast(m_Broadcast), error);
			if (error)
			{
				MS_ERROR("failed to set broadcast option: %s", error.message().c_str());
				break;
			}
			if (m_Multicast)
			{
				error = server.set_option(udp::socket::reuse_address(true), error);
				if (error)
				{
					MS_ERROR("failed to set reuse_address option: %s", error.message().c_str());
					break;
				}
				error = server.set_option(multicast::join_group(endpoint.address()), error);
				if (error)
				{
					MS_ERROR("failed to set multicast option: %s", error.message().c_str());
					break;
				}
			}
			error = server.bind(endpoint, error);
			if (error)
			{
				MS_ERROR("failed to bind: %s", error.message().c_str());
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
				if (localAddress == nullptr)
				{
					error = server.shutdown(tcp::socket::shutdown_both, error);
					if (error) MS_ERROR("failed to shutdown: %s", error.message().c_str());
					error = server.close(error);
					if (error) MS_ERROR("failed to close: %s", error.message().c_str());
					break;
				}
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			// Read and write data in async way

			MSLambda<void()> read_func;
			MSLambda<void(MSHnd<UDPChannel> channel, MSRef<IChannelEvent> event)> write_func;

			char buffer[2048];
			udp::endpoint remote;

			read_func = [&]()
			{
				if (server.is_open())
				{
					server.async_receive_from(asio::buffer(buffer), remote, [=, &read_func, &buffer, &server, &remote](asio::error_code error, size_t length) mutable
					{
						auto client = remote;

						// Get the actual ip and port number

						auto hashName = MSHash(remote.address().to_string() + ":" + std::to_string(remote.port()));
						auto channel = reactor->m_ChannelMap[hashName].lock();
						if (channel == nullptr)
						{
							MSRef<ISocketAddress> localAddress, remoteAddress;
							{
								auto address = server.local_endpoint().address().to_string();
								auto portNum = server.local_endpoint().port();
								auto family = server.local_endpoint().protocol().family();
								if (family == AF_INET) localAddress = MSNew<IPv4Address>(address, portNum);
								else if (family == AF_INET6) localAddress = MSNew<IPv6Address>(address, portNum);
								else MS_ERROR("unknown address family: %d", family);
							}
							{
								auto address = client.address().to_string();
								auto portNum = client.port();
								auto family = client.protocol().family();
								if (family == AF_INET) remoteAddress = MSNew<IPv4Address>(address, portNum);
								else if (family == AF_INET6) remoteAddress = MSNew<IPv6Address>(address, portNum);
								else MS_ERROR("unknown address family: %d", family);
							}
							if (localAddress == nullptr || remoteAddress == nullptr)
							{
								read_func();
								return;
							}

							channel = MSNew<UDPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), client);
							reactor->onConnect(channel);
						}

						if (error)
						{
							MS_ERROR("can't read from socket: %s", error.message().c_str());
							reactor->onDisconnect(channel);
						}
						else
						{
							auto event = MSNew<IChannelEvent>();
							event->Message = MSString(buffer, length);
							event->Channel = channel;
							reactor->onInbound(event);
							read_func();
						}
					});
				}
			};

			write_func = [&](MSHnd<UDPChannel> channel, MSRef<IChannelEvent> event)
			{
				auto client = channel.lock();
				if (server.is_open() && client)
				{
					server.async_send_to(asio::buffer(event->Message), client->getEndpoint(), [=, &write_func](asio::error_code error, size_t length) mutable
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
								event = reactor->m_EventQueue.front();
								reactor->m_EventQueue.pop();
								write_func(MSCast<UDPChannel>(event->Channel.lock()), event);
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

			read_func();

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
				auto channels = m_Channels;
				for (auto channel : channels)
				{
					if (channel) onDisconnect(channel);
				}
				m_Channels.clear();
				m_ChannelMap.clear();
			}

			MS_PRINT("closed server");
			return;

		} while (false);
		promise.set_value();
	});

	future.wait();
}

void UDPServerReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();

	m_Channels.clear();
	m_ChannelMap.clear();
}

MSHnd<IChannelAddress> UDPServerReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void UDPServerReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false || event == nullptr) return;
	auto channel = event->Channel.lock();
	if (channel && channel->running()) channel->write(event);
}

void UDPServerReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_Channels.insert(m_Channels.begin(), channel);
	m_ChannelMap[hashName] = channel;
	if (m_Backlog < m_Channels.size()) onDisconnect(m_Channels.back());
	ChannelReactor::onConnect(channel);
}

void UDPServerReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelMap.erase(hashName);
	m_Channels.erase(std::remove(m_Channels.begin(), m_Channels.end(), channel), m_Channels.end());
	ChannelReactor::onDisconnect(channel);
}

void UDPServerReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
	if (m_Sending == false) m_FireAsync();
}