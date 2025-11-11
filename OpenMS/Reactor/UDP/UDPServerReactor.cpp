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

	m_EventThread = MSThread([=, &promise]()
	{
		uv_loop_t loop;
		uv_udp_t server;
		uv_async_t async;

		uv_loop_init(&loop);
		uv_udp_init(&loop, &server);
		uv_async_init(&loop, &async, on_send);

		do
		{
			// Bind and listen to the socket

			if (true)
			{
				sockaddr_storage addr = {};
				uint32_t result = UV_EINVAL;
				if (auto ipv4 = MSCast<IPv4Address>(m_Address))
				{
					result = uv_ip4_addr(ipv4->getAddress().c_str(), ipv4->getPort(), (sockaddr_in*)&addr);
				}
				else if (auto ipv6 = MSCast<IPv6Address>(m_Address))
				{
					result = uv_ip6_addr(ipv6->getAddress().c_str(), ipv6->getPort(), (sockaddr_in6*)&addr);
				}
				if (result) MS_ERROR("invalid address: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_bind(&server, (sockaddr*)&addr, 0);
				if (result) MS_ERROR("bind error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_broadcast(&server, m_Broadcast ? 1 : 0);
				if (result) MS_ERROR("set broadcast error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_multicast_loop(&server, m_Multicast ? 1 : 0);
				if (result) MS_ERROR("set multicast loop error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			if (true)
			{
				sockaddr_storage addr = {};
				int addrLen = sizeof(addr);
				MSRef<ISocketAddress> localAddress;

				auto result = uv_udp_getsockname((uv_udp_t*)&server, (sockaddr*)&addr, &addrLen);
				if (result == 0)
				{
					if (addr.ss_family == AF_INET)
					{
						auto in_addr = (sockaddr_in*)&addr;
						char ip_str[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
						auto address = ip_str;
						auto portNum = ntohs(in_addr->sin_port);
						localAddress = MSNew<IPv4Address>(address, portNum);
					}
					else if (addr.ss_family == AF_INET6)
					{
						auto in6_addr = (sockaddr_in6*)&addr;
						char ip_str[INET6_ADDRSTRLEN];
						inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
						auto address = ip_str;
						auto portNum = ntohs(in6_addr->sin6_port);
						localAddress = MSNew<IPv6Address>(address, portNum);
					}
					else MS_ERROR("unknown address family: %d", addr.ss_family);
				}
				else MS_ERROR("failed to get socket name: %s", ::uv_strerror(result));

				if (localAddress == nullptr) break;
				m_LocalAddress = localAddress;

				MS_PRINT("listening on %s:%d", localAddress->getAddress().c_str(), localAddress->getPort());
			}

			// Start receiving data

			if (true)
			{
				auto result = uv_udp_recv_start(&server, on_alloc, on_read);
				if (result) MS_ERROR("recv start error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Run the event loop

			{
				loop.data = this;
				async.data = this;
				uv_timer_t timer;
				uv_timer_init(&loop, &timer);
				uv_timer_start(&timer, [](uv_timer_t* handle)
				{
					auto reactor = (UDPServerReactor*)handle->loop->data;
					if (reactor->m_Running == false) uv_stop(handle->loop);
				} , 500, 1);

				m_Connect = true;
				m_EventAsync = &async;
				promise.set_value();
				uv_run(&loop, UV_RUN_DEFAULT);
				m_EventAsync = nullptr;
				m_Connect = false;
			}

			// Close all channels

			if (true)
			{
				auto channels = m_Channels;
				for (auto channel : channels)
				{
					if (channel) onDisconnect(channel);
				}
				m_Channels.clear();
				m_ChannelMap.clear();
			}

			uv_close((uv_handle_t*)&server, nullptr);
			uv_loop_close(&loop);

			MS_PRINT("closed server");
			return;
		} while (false);

		uv_close((uv_handle_t*)&server, nullptr);
		uv_loop_close(&loop);
		promise.set_value();
		});

	future.wait();
}

void UDPServerReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_Channels.clear();

	for (auto& e : m_EventCache) if (e.second->Promise) e.second->Promise->set_value(false);
	m_EventCache.clear();
	m_ChannelMap.clear();
}

MSHnd<IChannelAddress> UDPServerReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
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
	uv_async_send(m_EventAsync);
}

void UDPServerReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void UDPServerReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* peer, unsigned flags)
{
	auto reactor = (UDPServerReactor*)req->loop->data;
	auto server = (uv_udp_t*)req;

	if (nread == 0 || peer == nullptr)
	{
		free(buf->base);
		return;
	}

	auto hashName = 0U;
	if (peer->sa_family == AF_INET)
	{
		auto in_addr = (sockaddr_in*)peer;
		char ip_str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
		auto address = MSString(ip_str);
		auto portNum = ntohs(in_addr->sin_port);
		hashName = MSHash(address + ":" + std::to_string(portNum));
	}
	else if (peer->sa_family == AF_INET6)
	{
		auto in6_addr = (sockaddr_in6*)peer;
		char ip_str[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
		auto address = MSString(ip_str);
		auto portNum = ntohs(in6_addr->sin6_port);
		hashName = MSHash(address + ":" + std::to_string(portNum));
	}
	else MS_ERROR("unknown address family: %d", peer->sa_family);

	auto channel = reactor->m_ChannelMap[hashName].lock();
	if (channel == nullptr)
	{
		// Get the actual ip and port number

		sockaddr_storage addr;
		int addrlen = sizeof(addr);
		MSRef<ISocketAddress> localAddress, remoteAddress;

		auto result = uv_udp_getsockname((uv_udp_t*)server, (sockaddr*)&addr, &addrlen);
		if (result == 0)
		{
			if (addr.ss_family == AF_INET)
			{
				auto in_addr = (sockaddr_in*)&addr;
				char ip_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
				auto address = ip_str;
				auto portNum = ntohs(in_addr->sin_port);
				localAddress = MSNew<IPv4Address>(address, portNum);
			}
			else if (addr.ss_family == AF_INET6)
			{
				auto in6_addr = (sockaddr_in6*)&addr;
				char ip_str[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
				auto portNum = ntohs(in6_addr->sin6_port);
				auto address = ip_str;
				localAddress = MSNew<IPv6Address>(address, portNum);
			}
			else MS_ERROR("unknown address family: %d", addr.ss_family);
		}
		else MS_ERROR("failed to get socket name: %s", ::uv_strerror(result));

		if (true)
		{
			if (peer->sa_family == AF_INET)
			{
				auto in_addr = (sockaddr_in*)peer;
				char ip_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
				auto address = ip_str;
				auto portNum = ntohs(in_addr->sin_port);
				remoteAddress = MSNew<IPv4Address>(address, portNum);
			}
			else if (peer->sa_family == AF_INET6)
			{
				auto in6_addr = (sockaddr_in6*)peer;
				char ip_str[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
				auto address = ip_str;
				auto portNum = ntohs(in6_addr->sin6_port);
				remoteAddress = MSNew<IPv6Address>(address, portNum);
			}
			else MS_ERROR("unknown address family: %d", peer->sa_family);
		}
		else MS_ERROR("failed to get socket name: %s", ::uv_strerror(result));

		if (localAddress == nullptr || remoteAddress == nullptr)
		{
			free(buf->base);
			return;
		}

		channel = MSNew<UDPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), server);
		server->data = channel.get();
		reactor->onConnect(channel);

		MS_DEBUG("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());
	}

	if (nread < 0 || channel->running() == false)
	{
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	auto event = MSNew<IChannelEvent>();
	event->Message = MSString((char*)buf->base, nread);
	event->Channel = channel->weak_from_this();
	reactor->onInbound(event);

	free(buf->base);
}

void UDPServerReactor::on_send(uv_async_t* handle)
{
	auto reactor = (UDPServerReactor*)handle->loop->data;
	if (reactor->m_Running == false || reactor->m_Sending == false) return;

	MSMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();
		reactor->m_EventCache[event.get()] = event;

		auto channel = MSCast<UDPChannel>(event->Channel.lock());
		if (channel == nullptr) continue;
		if (channel->running() == false) reactor->onDisconnect(channel);
		if (channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto server = channel->getHandle();
		auto remote = channel->getRemote().lock();

		sockaddr_storage addr = {};
		int result = UV_EINVAL;
		if (MSCast<IPv4Address>(remote))
		{
			auto _remote = MSCast<IPv4Address>(remote);
			auto portNum = _remote->getPort();
			auto address = _remote->getAddress();
			result = uv_ip4_addr(address.c_str(), portNum, (sockaddr_in*)&addr);
		}
		else if (MSCast<IPv6Address>(remote))
		{
			auto _remote = MSCast<IPv4Address>(remote);
			auto portNum = _remote->getPort();
			auto address = _remote->getAddress();
			result = uv_ip6_addr(address.c_str(), portNum, (sockaddr_in6*)&addr);
		}
		if (result) MS_ERROR("invalid address: %s", ::uv_strerror(result));
		if (result) continue;

		auto req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
		req->data = event.get();
		auto buf = uv_buf_init(event->Message.data(), (uint32_t)event->Message.size());
		result = uv_udp_send(req, server, &buf, 1, (sockaddr*)&addr, [](uv_udp_send_t* req, int status)
		{
			auto event = (IChannelEvent*)req->data;
			auto reactor = (UDPServerReactor*)req->handle->loop->data;
			auto channel = MSCast<UDPChannel>(event->Channel.lock());
			free(req);

			if (event->Promise) event->Promise->set_value(status == 0);
			reactor->m_EventCache.erase(event);

			if (channel && status)
			{
				MS_ERROR("write error: %s", uv_strerror(status));
				reactor->onDisconnect(channel);
			}
		});
		if (result)
		{
			free(req);

			if (event->Promise) event->Promise->set_value(false);
			reactor->m_EventCache.erase(event.get());

			MS_ERROR("write error: %s", uv_strerror(result));
			reactor->onDisconnect(channel);
		}
	}
}