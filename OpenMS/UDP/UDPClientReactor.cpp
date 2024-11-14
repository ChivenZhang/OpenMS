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
#include "UDPClientReactor.h"
#include "UDPChannel.h"

UDPClientReactor::UDPClientReactor(TRef<ISocketAddress> address, bool broadcast, bool multicast, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Broadcast(broadcast),
	m_Multicast(multicast),
	m_Address(address),
	m_AsyncStop(uv_async_t())
{
	if (m_Address == nullptr) m_Address = TNew<IPv4Address>("0.0.0.0", 0);
}

void UDPClientReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	TPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = TThread([=, &promise]() {
		uv_loop_t loop;
		uv_udp_t client;

		uv_loop_init(&loop);
		uv_udp_init(&loop, &client);
		uv_async_init(&loop, &m_AsyncStop, on_stop);

		do
		{
			// Bind and listen to the socket

			{
				sockaddr_storage addr;
				uint32_t result = uv_errno_t::UV_EINVAL;
				if (auto ipv4 = TCast<IPv4Address>(m_Address))
				{
					result = uv_ip4_addr(ipv4->getAddress().c_str(), ipv4->getPort(), (sockaddr_in*)&addr);
				}
				else if (auto ipv6 = TCast<IPv6Address>(m_Address))
				{
					result = uv_ip6_addr(ipv6->getAddress().c_str(), ipv6->getPort(), (sockaddr_in6*)&addr);
				}
				if (result) TError("invalid address: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_connect(&client, (sockaddr*)&addr);
				if (result) TError("connect error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_broadcast(&client, m_Broadcast ? 1 : 0);
				if (result) TError("set broadcast error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_multicast_loop(&client, m_Multicast ? 1 : 0);
				if (result) TError("set multicast loop error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			{
				sockaddr_storage addr;
				socklen_t addrlen = sizeof(addr);
				TRef<ISocketAddress> localAddress, remoteAddress;

				auto result = uv_udp_getsockname((uv_udp_t*)&client, (sockaddr*)&addr, &addrlen);
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

				result = uv_udp_getpeername((uv_udp_t*)&client, (sockaddr*)&addr, &addrlen);
				if (result == 0)
				{
					if (addr.ss_family == AF_INET)
					{
						auto in_addr = (sockaddr_in*)&addr;
						char ip_str[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
						auto address = ip_str;
						auto portNum = ntohs(in_addr->sin_port);
						remoteAddress = TNew<IPv4Address>(address, portNum);
					}
					else if (addr.ss_family == AF_INET6)
					{
						auto in6_addr = (sockaddr_in6*)&addr;
						char ip_str[INET6_ADDRSTRLEN];
						inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
						auto portNum = ntohs(in6_addr->sin6_port);
						auto address = ip_str;
						remoteAddress = TNew<IPv6Address>(address, portNum);
					}
					else TError("unknown address family: %d", addr.ss_family);
				}
				else TError("failed to get socket name: %s", ::uv_strerror(result));

				if (localAddress == nullptr || remoteAddress == nullptr) break;

				auto channel = TNew<UDPChannel>(this, localAddress, remoteAddress, &client);
				client.data = channel.get();
				onConnect(channel);

				TDebug("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());
			}

			// Start receiving data

			{
				auto result = uv_udp_recv_start(&client, on_alloc, on_read);
				if (result) TError("recv start error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			{
				sockaddr_storage addr;
				socklen_t addrlen = sizeof(addr);
				TRef<ISocketAddress> localAddress;

				auto result = uv_udp_getsockname((uv_udp_t*)&client, (sockaddr*)&addr, &addrlen);
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

			// Run the event loop

			{
				loop.data = this;
				promise.set_value();

				while (m_Running == true && uv_run(&loop, UV_RUN_NOWAIT)) on_send(&client);
			}

		} while (0);

		TPrint("closing client");

		uv_close((uv_handle_t*)&m_AsyncStop, nullptr);
		uv_close((uv_handle_t*)&client, nullptr);
		uv_loop_close(&loop);
		});

	future.wait();
}

void UDPClientReactor::shutdown()
{
	if (m_Running == false) return;
	uv_async_send(&m_AsyncStop);
	ChannelReactor::shutdown();
	m_Channel = nullptr;
}

void UDPClientReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->write(event);
}

void UDPClientReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->writeAndFlush(event);
}

void UDPClientReactor::onConnect(TRef<Channel> channel)
{
	ChannelReactor::onConnect(channel);
	m_Channel = channel;
}

void UDPClientReactor::onDisconnect(TRef<Channel> channel)
{
	ChannelReactor::onDisconnect(channel);
	m_Channel = nullptr;
}

void UDPClientReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void UDPClientReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* peer, unsigned flags)
{
	auto reactor = (UDPClientReactor*)req->loop->data;
	auto channel = reactor->m_Channel;

	if (nread == 0 || peer == nullptr) return;

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

void UDPClientReactor::on_send(uv_udp_t* handle)
{
	auto reactor = (UDPClientReactor*)handle->loop->data;
	auto client = (uv_udp_t*)handle;

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
		auto client = channel->getHandle();

		size_t sentNum = 0;
		while (sentNum < event->Message.size())
		{
			auto buf = uv_buf_init(event->Message.data() + sentNum, (unsigned)(event->Message.size() - sentNum));
			auto result = uv_udp_try_send(client, &buf, 1, nullptr);
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

void UDPClientReactor::on_stop(uv_async_t* handle)
{
	uv_stop(handle->loop);
}