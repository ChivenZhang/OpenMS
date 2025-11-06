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
		uv_loop_t loop;
		uv_udp_t client;

		uv_loop_init(&loop);
		uv_udp_init(&loop, &client);

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

				result = uv_udp_connect(&client, (sockaddr*)&addr);
				if (result) MS_ERROR("connect error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_broadcast(&client, m_Broadcast ? 1 : 0);
				if (result) MS_ERROR("set broadcast error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_udp_set_multicast_loop(&client, m_Multicast ? 1 : 0);
				if (result) MS_ERROR("set multicast loop error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			if (true)
			{
				sockaddr_storage addr = {};
				int addrLen = sizeof(addr);
				MSRef<ISocketAddress> localAddress, remoteAddress;

				auto result = uv_udp_getsockname((uv_udp_t*)&client, (sockaddr*)&addr, &addrLen);
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

				result = uv_udp_getpeername(&client, (sockaddr*)&addr, &addrLen);
				if (result == 0)
				{
					if (addr.ss_family == AF_INET)
					{
						auto in_addr = (sockaddr_in*)&addr;
						char ip_str[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
						auto address = ip_str;
						auto portNum = ntohs(in_addr->sin_port);
						remoteAddress = MSNew<IPv4Address>(address, portNum);
					}
					else if (addr.ss_family == AF_INET6)
					{
						auto in6_addr = (sockaddr_in6*)&addr;
						char ip_str[INET6_ADDRSTRLEN];
						inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
						auto portNum = ntohs(in6_addr->sin6_port);
						auto address = ip_str;
						remoteAddress = MSNew<IPv6Address>(address, portNum);
					}
					else MS_ERROR("unknown address family: %d", addr.ss_family);
				}
				else MS_ERROR("failed to get socket name: %s", ::uv_strerror(result));

				if (localAddress == nullptr || remoteAddress == nullptr) break;
				m_LocalAddress = localAddress;

				auto channel = MSNew<UDPChannel>(this, localAddress, remoteAddress, (uint32_t)(rand() % m_WorkerList.size()), &client);
				client.data = channel.get();
				onConnect(channel);

				MS_PRINT("listening on %s:%d", localAddress->getAddress().c_str(), localAddress->getPort());
			}

			// Start receiving data

			if (true)
			{
				auto result = uv_udp_recv_start(&client, on_alloc, on_read);
				if (result) MS_ERROR("recv start error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Run the event loop

			{
				loop.data = this;
				uv_timer_t timer;
				uv_timer_init(&loop, &timer);
				uv_timer_start(&timer, on_send, 0, 1);

				m_Connect = true;
				promise.set_value();
				uv_run(&loop, UV_RUN_DEFAULT);
				m_Connect = false;
			}

			// Close all channels

			if (true)
			{
				if (m_Channel) onDisconnect(m_Channel);
				m_Channel = nullptr;
			}

			uv_close((uv_handle_t*)&client, nullptr);
			uv_loop_close(&loop);

			MS_PRINT("closed client");
			return;
		} while (false);

		uv_close((uv_handle_t*)&client, nullptr);
		uv_loop_close(&loop);
		promise.set_value();
		});

	future.wait();
}

void UDPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_Channel = nullptr;

	for (auto& e : m_EventCache) if (e.second->Promise) e.second->Promise->set_value(false);
	m_EventCache.clear();
}

MSHnd<IChannelAddress> UDPClientReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void UDPClientReactor::write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address)
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

void UDPClientReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void UDPClientReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* peer, unsigned flags)
{
	auto reactor = (UDPClientReactor*)req->loop->data;
	auto channel = reactor->m_Channel;

	if (nread == 0 || peer == nullptr)
	{
		free(buf->base);
		return;
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

void UDPClientReactor::on_send(uv_timer_t* handle)
{
	auto reactor = (UDPClientReactor*)handle->loop->data;
	if (reactor->m_Running == false) uv_stop(handle->loop);
	if (reactor->m_Running == false || reactor->m_Sending == false) return;

	MSMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.empty() == false)
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();
		reactor->m_EventCache[event.get()] = event;

		auto channel = MSCast<UDPChannel>(event->Channel.lock());
		if (channel == nullptr) continue;
		if (channel->running() == false) reactor->onDisconnect(channel);
		if (channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto client = channel->getHandle();

		auto req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
		req->data = event.get();
		auto buf = uv_buf_init(event->Message.data(), (uint32_t)event->Message.size());
		auto result = uv_udp_send(req, client, &buf, 1, nullptr, [](uv_udp_send_t* req, int status)
		{
			auto event = (IChannelEvent*)req->data;
			auto reactor = (UDPClientReactor*)req->handle->loop->data;
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