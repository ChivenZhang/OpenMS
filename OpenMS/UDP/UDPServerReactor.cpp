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
#include "UDPServerReactor.h"
#include "UDPChannel.h"

UDPServerReactor::UDPServerReactor(TRef<ISocketAddress> address, bool broadcast, bool multicast, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Broadcast(broadcast),
	m_Multicast(multicast),
	m_SocketAddress(address),
	m_AsyncStop(uv_async_t())
{
	if (m_SocketAddress == nullptr) m_SocketAddress = TNew<IPv4Address>("0.0.0.0", 0);
	m_Address = m_SocketAddress->getAddress();
	m_PortNum = m_SocketAddress->getPort();
}

void UDPServerReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	TPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = TThread([=, &promise]() {
		uv_loop_t loop;
		uv_udp_t server;

		uv_loop_init(&loop);
		uv_udp_init(&loop, &server);
		uv_async_init(&loop, &m_AsyncStop, on_stop);

		do
		{
			// Bind and listen to the socket

			{
				sockaddr_storage addr;
				uint32_t result = uv_errno_t::UV_EINVAL;
				if (TCast<IPv4Address>(m_SocketAddress))
				{
					result = uv_ip4_addr(m_Address.c_str(), m_PortNum, (sockaddr_in*)&addr);
				}
				else if (TCast<IPv6Address>(m_SocketAddress))
				{
					result = uv_ip6_addr(m_Address.c_str(), m_PortNum, (sockaddr_in6*)&addr);
				}
				if (result) TError("invalid address: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_bind(&server, (sockaddr*)&addr, 0);
				if (result) TError("bind error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_broadcast(&server, m_Broadcast ? 1 : 0);
				if (result) TError("set broadcast error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_multicast_loop(&server, m_Multicast ? 1 : 0);
				if (result) TError("set multicast loop error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			{
				sockaddr_storage addr;
				socklen_t addrlen = sizeof(addr);
				TRef<ISocketAddress> localAddress;

				auto result = uv_udp_getsockname((uv_udp_t*)&server, (sockaddr*)&addr, &addrlen);
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
						auto address = ip_str;
						auto portNum = ntohs(in6_addr->sin6_port);
						localAddress = TNew<IPv6Address>(address, portNum);
					}
					else TError("unknown address family: %d", addr.ss_family);
				}
				else TError("failed to get socket name: %s", ::uv_strerror(result));

				if (localAddress == nullptr) break;

				TPrint("listening on %s:%d", localAddress->getAddress().c_str(), localAddress->getPort());
			}

			{
				auto result = uv_udp_recv_start(&server, on_alloc, on_read);
				if (result) TError("recv start error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Run the event loop

			{
				loop.data = this;
				promise.set_value();

				while (m_Running == true && uv_run(&loop, UV_RUN_NOWAIT)) on_send(&server);
			}

		} while (0);

		TPrint("closing server");

		uv_close((uv_handle_t*)&m_AsyncStop, nullptr);
		uv_close((uv_handle_t*)&server, nullptr);
		uv_loop_close(&loop);
		});
	future.wait();
}

void UDPServerReactor::shutdown()
{
	if (m_Running == false) return;
	uv_async_send(&m_AsyncStop);
	ChannelReactor::shutdown();
}

void UDPServerReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

void UDPServerReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

void UDPServerReactor::onConnect(TRef<Channel> channel)
{
	ChannelReactor::onConnect(channel);
	auto hashName = THash(channel->getRemote()->getAddress());
	m_Connections[hashName] = channel;
}

void UDPServerReactor::onDisconnect(TRef<Channel> channel)
{
	ChannelReactor::onDisconnect(channel);
	auto hashName = THash(channel->getRemote()->getAddress());
	m_Connections.erase(hashName);
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

		channel = TNew<UDPChannel>(reactor, localAddress, remoteAddress, server);
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

void UDPServerReactor::on_send(uv_udp_t* handle)
{
	auto reactor = (UDPServerReactor*)handle->loop->data;

	if (reactor->m_Sending == false) return;

	TMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();

		if (event->Channel.expired()) continue;
		auto channel = TCast<UDPChannel>(event->Channel.lock());
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

void UDPServerReactor::on_stop(uv_async_t* handle)
{
	uv_stop(handle->loop);
}