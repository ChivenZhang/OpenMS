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
#include "KCPServerReactor.h"
#include "KCPChannel.h"
#include <asio.hpp>

namespace
{
	IUINT32 iclock()
	{
		return (IUINT32)(std::chrono::steady_clock::now().time_since_epoch().count() / 1000000);
	}
}

KCPServerReactor::KCPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_kcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Backlog(backlog ? backlog : 128),
	m_Session(1),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
	if (m_OnSession == nullptr) m_OnSession = [this](MSRef<IChannelAddress>) { return m_Session++; };
}

void KCPServerReactor::startup()
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
				if (localAddress == nullptr) break;
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			// Read and write data in async way

			char buffer[2048];
			char buffer2[2048];
			udp::endpoint remote;

			MSLambda<void()> read_func;
			read_func = [&]()
			{
				if (server.is_open())
				{
					server.async_receive_from(asio::buffer(buffer), remote, [=, &read_func, &server, &buffer2](asio::error_code error, size_t length)
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

							auto session = ikcp_create(reactor->m_OnSession(remoteAddress), nullptr);
							channel = MSNew<KCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), session, &server, client);

							session->user = channel.get();
							ikcp_setoutput(session, on_output);
							ikcp_nodelay(session, 1, 5, 1, 1);
							// Pass the session id to remote client
							asio::error_code result;
							server.send_to(asio::buffer(session, sizeof(uint32_t)), client, 0, result);
							if (!result) reactor->onConnect(channel);

							read_func();
							return;
						}

						if (error)
						{
							MS_ERROR("can't read from socket: %s", error.message().c_str());
							reactor->onDisconnect(channel);
						}
						else
						{
							auto _channel = MSCast<KCPChannel>(channel);
							auto session = _channel->getSession();
							ikcp_input(session, buffer, (long)length);

							auto length2 = ikcp_recv(session, buffer2, sizeof(buffer2));
							while (0 < length2)
							{
								auto event = MSNew<IChannelEvent>();
								event->Message = MSStringView(buffer2, length2);
								event->Channel = channel;
								reactor->onInbound(event);

								length2 = ikcp_recv(session, buffer2, sizeof(buffer2));
							}

							read_func();
						}
					});
				}
			};
			read_func();

			asio::steady_timer ticker(loop);
			MSLambda<void()> tick_func;
			tick_func = [&]()
			{
				ticker.async_wait([&](asio::error_code)
				{
					reactor->m_ChannelsRemoved.clear();

					// Update all kcp sessions

					for (size_t i = 0; i < reactor->m_Channels.size(); ++i)
					{
						auto channel = MSCast<KCPChannel>(reactor->m_Channels[i]);
						if (channel == nullptr || channel->running() == false) continue;
						auto session = channel->getSession();

						auto nowTime = iclock();
						auto nextTime = ikcp_check(session, nowTime);
						if (nextTime <= nowTime) ikcp_update(session, nowTime);

						auto length2 = ikcp_recv(session, buffer2, sizeof(buffer2));
						while (0 < length2)
						{
							auto event = MSNew<IChannelEvent>();
							event->Message = MSStringView(buffer2, length2);
							event->Channel = channel;
							reactor->onInbound(event);

							length2 = ikcp_recv(session, buffer2, sizeof(buffer2));
						}
					}

					MSMutexLock lock(reactor->m_EventLock);
					while (reactor->m_EventQueue.size())
					{
						auto event = reactor->m_EventQueue.front();
						reactor->m_EventQueue.pop();

						auto result = -1;
						do
						{
							auto channel = MSCast<KCPChannel>(event->Channel.lock());
							if (channel == nullptr || channel->running() == false) break;
							if (event->Message.empty()) break;
							auto session = channel->getSession();

							result = ikcp_send(session, event->Message.data(), (int)event->Message.size());
							if (result < 0) reactor->onDisconnect(channel);
						} while (false);

						if (event->Promise) event->Promise->set_value(result == 0);
					}

					tick_func();
				});
			};
			tick_func();

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

			// Close all channels

			{
				auto channels = m_Channels;
				for (auto channel : channels)
				{
					if (channel) onDisconnect(channel);
				}
				m_Channels.clear();
				m_ChannelMap.clear();
				m_ChannelsRemoved.clear();
			}

			if (loop.stopped() == false) loop.stop();
			MS_INFO("closed server");
			return;

		} while (false);
		promise.set_value();
	});

	future.wait();
}

void KCPServerReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();

	m_Channels.clear();
	m_ChannelMap.clear();
	m_ChannelsRemoved.clear();
}

MSHnd<IChannelAddress> KCPServerReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void KCPServerReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false || event == nullptr) return;
	auto channel = event->Channel.lock();
	if (channel && channel->running()) channel->write(event);
}

void KCPServerReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_Channels.insert(m_Channels.begin(), channel);
	m_ChannelMap[hashName] = channel;
	if (m_Backlog < m_Channels.size()) onDisconnect(m_Channels.back());
	ChannelReactor::onConnect(channel);
}

void KCPServerReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelsRemoved.push_back(channel);
	m_ChannelMap.erase(hashName);
	m_Channels.erase(std::remove(m_Channels.begin(), m_Channels.end(), channel), m_Channels.end());
	ChannelReactor::onDisconnect(channel);
}

void KCPServerReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
}

int KCPServerReactor::on_output(const char* buf, int len, IKCPCB* kcp, void* user)
{
	auto channel = (KCPChannel*)user;
	auto socket = channel->getSocket();
	auto reactor = MSCast<KCPServerReactor>(channel->getReactor());
	auto& remote = channel->getEndpoint();
	asio::error_code error;
	socket->send_to(asio::buffer(buf, len), remote, 0, error);
	if (error)
	{
		reactor->onDisconnect(channel->shared_from_this());
		return -1;
	}
	return 0;
}