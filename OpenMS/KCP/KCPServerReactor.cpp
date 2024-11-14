/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "KCPServerReactor.h"
#include "KCPChannel.h"

KCPServerReactor::KCPServerReactor(TRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Backlog(backlog ? backlog : 128),
	m_SocketAddress(address),
	m_AsyncStop(uv_async_t())
{
	if (m_SocketAddress == nullptr) m_SocketAddress = TNew<IPv4Address>("0.0.0.0", 0);
}

void KCPServerReactor::startup()
{
}

void KCPServerReactor::shutdown()
{
	if (m_Running == false) return;
	uv_async_send(&m_AsyncStop);
	ChannelReactor::shutdown();
}

void KCPServerReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto remote = TCast<ISocketAddress>(address);
	if (remote == nullptr) return;
	auto hashName = THash(remote->getAddress() + ":" + std::to_string(remote->getPort()));
	auto result = m_Connections.find(hashName);
	if (result == m_Connections.end() && result->second == nullptr) return;
	auto channel = result->second;
	channel->write(event);
}

void KCPServerReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto remote = TCast<ISocketAddress>(address);
	if (remote == nullptr) return;
	auto hashName = THash(remote->getAddress() + ":" + std::to_string(remote->getPort()));
	auto result = m_Connections.find(hashName);
	if (result == m_Connections.end() && result->second == nullptr) return;
	auto channel = result->second;
	channel->writeAndFlush(event);
}

void KCPServerReactor::onConnect(TRef<Channel> channel)
{
	ChannelReactor::onConnect(channel);
	auto remote = TCast<ISocketAddress>(channel->getRemote());
	auto hashName = THash(remote->getAddress() + ":" + std::to_string(remote->getPort()));
	m_Channels.insert(m_Channels.begin(), channel);
	m_Connections[hashName] = channel;
	if (m_Backlog < m_Channels.size())
	{
		auto channel = m_Channels.back();
		auto remote = TCast<ISocketAddress>(channel->getRemote());
		auto hashName = THash(remote->getAddress() + ":" + std::to_string(remote->getPort()));
		m_Channels.pop_back();
		m_Connections.erase(hashName);
	}
}

void KCPServerReactor::onDisconnect(TRef<Channel> channel)
{
	ChannelReactor::onDisconnect(channel);
	auto remote = TCast<ISocketAddress>(channel->getRemote());
	auto hashName = THash(remote->getAddress() + ":" + std::to_string(remote->getPort()));
	m_Connections.erase(hashName);
	auto result = std::find(m_Channels.begin(), m_Channels.end(), channel);
	if (result != m_Channels.end()) m_Channels.erase(result);
}

void KCPServerReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void KCPServerReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* peer, unsigned flags)
{
	auto reactor = (KCPServerReactor*)req->loop->data;
	auto server = (uv_udp_t*)req;

	if (nread == 0 || peer == nullptr) return;

	auto hashName = 0U;
	if (peer->sa_family == AF_INET)
	{
		auto in_addr = (sockaddr_in*)peer;
		char ip_str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
		auto address = TString(ip_str);
		auto portNum = ntohs(in_addr->sin_port);
		hashName = THash(address + ":" + std::to_string(portNum));
	}
	else if (peer->sa_family == AF_INET6)
	{
		auto in6_addr = (sockaddr_in6*)peer;
		char ip_str[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
		auto address = TString(ip_str);
		auto portNum = ntohs(in6_addr->sin6_port);
		hashName = THash(address + ":" + std::to_string(portNum));
	}
	else TError("unknown address family: %d", peer->sa_family);

	auto channel = reactor->m_Connections[hashName];
	if (channel == nullptr)
	{
		// Get the actual ip and port number

		sockaddr_storage addr;
		socklen_t addrlen = sizeof(addr);
		TRef<ISocketAddress> localAddress, remoteAddress;

		auto result = uv_tcp_getsockname((uv_tcp_t*)server, (sockaddr*)&addr, &addrlen);
		if (result == 0)
		{
			if (addr.ss_family == AF_INET)
			{
				auto in_addr = (sockaddr_in*)&addr;
				char ip_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
				auto address = ip_str;
				auto portNum = ntohs(in_addr->sin_port);
				localAddress = TNew<IPv4Address>(address, portNum);
			}
			else if (addr.ss_family == AF_INET6)
			{
				auto in6_addr = (sockaddr_in6*)&addr;
				char ip_str[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
				auto portNum = ntohs(in6_addr->sin6_port);
				auto address = ip_str;
				localAddress = TNew<IPv6Address>(address, portNum);
			}
			else TError("unknown address family: %d", addr.ss_family);
		}
		else TError("failed to get socket name: %s", ::uv_strerror(result));

		if (true)
		{
			if (peer->sa_family == AF_INET)
			{
				auto in_addr = (sockaddr_in*)peer;
				char ip_str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
				auto address = ip_str;
				auto portNum = ntohs(in_addr->sin_port);
				remoteAddress = TNew<IPv4Address>(address, portNum);
			}
			else if (peer->sa_family == AF_INET6)
			{
				auto in6_addr = (sockaddr_in6*)peer;
				char ip_str[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
				auto address = ip_str;
				auto portNum = ntohs(in6_addr->sin6_port);
				remoteAddress = TNew<IPv6Address>(address, portNum);
			}
			else TError("unknown address family: %d", peer->sa_family);
		}
		else TError("failed to get socket name: %s", ::uv_strerror(result));

		if (localAddress == nullptr || remoteAddress == nullptr)
		{
			free(buf->base);
			return;
		}

		channel = TNew<KCPChannel>(reactor, localAddress, remoteAddress, server, nullptr);
		server->data = channel.get();
		reactor->onConnect(channel);

		TDebug("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());
	}

	if (nread < 0)
	{
		reactor->onDisconnect(channel);

		free(buf->base);
		return;
	}

	auto event = TNew<IChannelEvent>();
	event->Message = TString((char*)buf->base, nread);
	event->Channel = channel->weak_from_this();
	reactor->onInbound(event);

	free(buf->base);
}

void KCPServerReactor::on_send(uv_udp_t* handle)
{
	auto reactor = (KCPServerReactor*)handle->loop->data;

	if (reactor->m_Sending == false) return;

	TMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();

		if (event->Channel.expired()) continue;
		auto channel = TCast<KCPChannel>(event->Channel.lock());
		if (channel == nullptr || channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto server = channel->getHandle();
		auto remote = channel->getRemote();

		sockaddr_storage addr = {};
		int result = uv_errno_t::UV_EINVAL;
		if (TCast<IPv4Address>(remote))
		{
			auto _remote = TCast<IPv4Address>(remote);
			auto portNum = _remote->getPort();
			auto address = _remote->getAddress();
			result = uv_ip4_addr(address.c_str(), portNum, (sockaddr_in*)&addr);
		}
		else if (TCast<IPv6Address>(remote))
		{
			auto _remote = TCast<IPv4Address>(remote);
			auto portNum = _remote->getPort();
			auto address = _remote->getAddress();
			result = uv_ip6_addr(address.c_str(), portNum, (sockaddr_in6*)&addr);
		}
		if (result) TError("invalid address: %s", ::uv_strerror(result));
		if (result) continue;

		size_t sentNum = 0;
		while (sentNum < event->Message.size())
		{
			auto buf = uv_buf_init(event->Message.data() + sentNum, (unsigned)(event->Message.size() - sentNum));
			result = uv_udp_try_send(server, &buf, 1, (sockaddr*)&addr);
			if (result < 0)
			{
				reactor->onDisconnect(channel);
				break;
			}
			else if (result == UV_EAGAIN) continue;
			else sentNum += result;
		}
		if (event->Promise)
		{
			event->Promise->set_value(sentNum == event->Message.size());
		}
	}
}

void KCPServerReactor::on_stop(uv_async_t* handle)
{
	uv_stop(handle->loop);
}