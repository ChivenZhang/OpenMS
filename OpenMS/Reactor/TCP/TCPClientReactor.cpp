#include "TCPClientReactor.h"
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
		uv_loop_t loop;
		uv_tcp_t client;
		uv_async_t async;
		uv_connect_t connect_req;

		uv_loop_init(&loop);
		uv_tcp_init(&loop, &client);
		uv_async_init(&loop, &async, on_send);

		do
		{
			// Set up the client

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

				result = uv_tcp_connect(&connect_req, &client, (const struct sockaddr*)&addr, on_connect);
				if (result) MS_ERROR("failed to connect: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			{
				sockaddr_storage addr = {};
				int addrLen = sizeof(addr);
				MSRef<ISocketAddress> localAddress;

				auto result = uv_tcp_getsockname((uv_tcp_t*)&client, (struct sockaddr*)&addr, &addrLen);
				if (result == 0)
				{
					if (addr.ss_family == AF_INET)
					{
						auto in_addr = (struct sockaddr_in*)&addr;
						char ip_str[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
						auto address = ip_str;
						auto portNum = ntohs(in_addr->sin_port);
						localAddress = MSNew<IPv4Address>(address, portNum);
					}
					else if (addr.ss_family == AF_INET6)
					{
						auto in6_addr = (struct sockaddr_in6*)&addr;
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

			// Run the event loop

			{
				loop.data = this;
				async.data = this;
				uv_timer_t timer;
				uv_timer_init(&loop, &timer);
				uv_timer_start(&timer, [](uv_timer_t* handle)
				{
					auto reactor = (TCPClientReactor*)handle->loop->data;
					if (reactor->m_Running == false) uv_stop(handle->loop);
				} , 1000, 1);

				m_EventAsync = &async;
				uv_run(&loop, UV_RUN_ONCE);
				promise.set_value();
				uv_run(&loop, UV_RUN_DEFAULT);
				m_EventAsync = nullptr;
				m_Connect = false;
			}

			// Close all channels

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

void TCPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_Channel = nullptr;

	for (auto& e : m_EventCache) if (e.second->Promise) e.second->Promise->set_value(false);
	m_EventCache.clear();
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
	uv_async_send(m_EventAsync);
}

void TCPClientReactor::on_connect(uv_connect_t* req, int status)
{
	auto reactor = (TCPClientReactor*)req->handle->loop->data;

	if (status < 0)
	{
		MS_ERROR("failed to connect: %s", ::uv_strerror(status));
		return;
	}

	// Get the actual ip and port number

	sockaddr_storage addr = {};
	int addrLen = sizeof(addr);
	MSRef<ISocketAddress> localAddress, remoteAddress;

	auto client = (uv_tcp_t*)req->handle;
	auto result = uv_tcp_getsockname((uv_tcp_t*)client, (struct sockaddr*)&addr, &addrLen);
	if (result == 0)
	{
		if (addr.ss_family == AF_INET)
		{
			auto in_addr = (struct sockaddr_in*)&addr;
			char ip_str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
			auto address = ip_str;
			auto portNum = ntohs(in_addr->sin_port);
			localAddress = MSNew<IPv4Address>(address, portNum);
		}
		else if (addr.ss_family == AF_INET6)
		{
			auto in6_addr = (struct sockaddr_in6*)&addr;
			char ip_str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
			auto portNum = ntohs(in6_addr->sin6_port);
			auto address = ip_str;
			localAddress = MSNew<IPv6Address>(address, portNum);
		}
		else MS_ERROR("unknown address family: %d", addr.ss_family);
	}
	else MS_ERROR("failed to get socket name: %s", ::uv_strerror(result));

	result = uv_tcp_getpeername((uv_tcp_t*)client, (struct sockaddr*)&addr, &addrLen);
	if (result == 0)
	{
		if (addr.ss_family == AF_INET)
		{
			auto in_addr = (struct sockaddr_in*)&addr;
			char ip_str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
			auto address = ip_str;
			auto portNum = ntohs(in_addr->sin_port);
			remoteAddress = MSNew<IPv4Address>(address, portNum);
		}
		else if (addr.ss_family == AF_INET6)
		{
			auto in6_addr = (struct sockaddr_in6*)&addr;
			char ip_str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
			auto address = ip_str;
			auto portNum = ntohs(in6_addr->sin6_port);
			remoteAddress = MSNew<IPv6Address>(address, portNum);
		}
		else MS_ERROR("unknown address family: %d", addr.ss_family);
	}
	else MS_ERROR("failed to get socket name: %s", ::uv_strerror(result));

	if (localAddress == nullptr || remoteAddress == nullptr)
	{
		return;
	}

	auto channel = MSNew<TCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), client);
	client->data = channel.get();
	reactor->onConnect(channel);

	// Start reading data from the server

	result = uv_read_start((uv_stream_t*)client, on_alloc, on_read);
	if (result)
	{
		MS_ERROR("readChannel start error: %s", ::uv_strerror(result));

		reactor->onDisconnect(channel->shared_from_this());
	}
}

void TCPClientReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void TCPClientReactor::on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	auto reactor = (TCPClientReactor*)stream->loop->data;
	auto channel = reactor->m_Channel;
	if (channel == nullptr) return;

	if (nread < 0 || channel->running() == false)
	{
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	printf("客户端接收：");
	for (auto i = 0; i < nread; ++i) printf("%u ", (uint8_t)buf->base[i]);
	printf("\n");

	// 处理接收到的数据
	auto event = MSNew<IChannelEvent>();
	event->Message = MSString(buf->base, nread);
	event->Channel = channel;
	reactor->onInbound(event);

	// 释放缓冲区
	free(buf->base);
}

void TCPClientReactor::on_send(uv_async_t* handle)
{
	auto reactor = (TCPClientReactor*)handle->loop->data;
	if (reactor->m_Running == false || reactor->m_Sending == false) return;

	MSMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();
		reactor->m_EventCache[event.get()] = event;

		if (event->Channel.expired()) continue;
		auto channel = MSCast<TCPChannel>(event->Channel.lock());
		if (channel == nullptr) continue;
		if (channel->running() == false) reactor->onDisconnect(channel);
		if (channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto client = channel->getHandle();

		auto req = (uv_write_t*)malloc(sizeof(uv_write_t));
		req->data = event.get();
		auto buf = uv_buf_init(event->Message.data(), (uint32_t)event->Message.size());

		printf("客户端发送：");
		for (auto i = 0; i < buf.len; ++i) printf("%u ", (uint8_t)buf.base[i]);
		printf("\n");

		auto result = uv_write(req, (uv_stream_t*)client, &buf, 1, [](uv_write_t* req, int status)
		{
			auto event = (IChannelEvent*)req->data;
			auto reactor = (TCPClientReactor*)req->handle->loop->data;
			auto channel = MSCast<TCPChannel>(event->Channel.lock());
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