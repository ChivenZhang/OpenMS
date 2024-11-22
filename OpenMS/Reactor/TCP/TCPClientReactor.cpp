#include "TCPClientReactor.h"
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "TCPClientReactor.h"
#include "TCPChannel.h"

TCPClientReactor::TCPClientReactor(TRef<ISocketAddress> address, size_t workerNum, callback_tcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(address)
{
	if (m_Address == nullptr) m_Address = TNew<IPv4Address>("0.0.0.0", 0);
}

void TCPClientReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	TPromise<void> promise;
	auto future = promise.get_future();
	m_EventThread = TThread([=, &promise]() {
		uv_loop_t loop;
		uv_tcp_t client;
		uv_connect_t connect_req;

		uv_loop_init(&loop);
		uv_tcp_init(&loop, &client);

		do
		{
			// Set up the client

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

				result = uv_tcp_connect(&connect_req, &client, (const struct sockaddr*)&addr, on_connect);
				if (result) TError("failed to connect: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			{
				sockaddr_storage addr;
				socklen_t addrlen = sizeof(addr);
				TRef<ISocketAddress> localAddress;

				auto result = uv_tcp_getsockname((uv_tcp_t*)&client, (struct sockaddr*)&addr, &addrlen);
				if (result == 0)
				{
					if (addr.ss_family == AF_INET)
					{
						auto in_addr = (struct sockaddr_in*)&addr;
						char ip_str[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
						auto address = ip_str;
						auto portNum = ntohs(in_addr->sin_port);
						localAddress = TNew<IPv4Address>(address, portNum);
					}
					else if (addr.ss_family == AF_INET6)
					{
						auto in6_addr = (struct sockaddr_in6*)&addr;
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
				m_LocalAddress = localAddress;

				TPrint("listening on %s:%d", localAddress->getAddress().c_str(), localAddress->getPort());
			}

			// Run the event loop

			{
				loop.data = this;
				m_Connect = true;
				promise.set_value();

				while (m_Running == true && uv_run(&loop, UV_RUN_NOWAIT))
				{
					on_send(&client);
				}
				m_Connect = false;
			}

			// Close all channels

			{
				if (m_Channel) onDisconnect(m_Channel);
				m_Channel = nullptr;
			}

		} while (0);

		uv_close((uv_handle_t*)&client, nullptr);
		uv_loop_close(&loop);

		TPrint("closed client");
		});

	future.wait();
}

void TCPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_Channel = nullptr;
}

THnd<IChannelAddress> TCPClientReactor::address() const
{
	return m_Connect ? m_LocalAddress : THnd<IChannelAddress>();
}

void TCPClientReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->write(event);
}

void TCPClientReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->writeAndFlush(event);
}

void TCPClientReactor::onConnect(TRef<Channel> channel)
{
	m_Channel = channel;
	ChannelReactor::onConnect(channel);
}

void TCPClientReactor::onDisconnect(TRef<Channel> channel)
{
	m_Channel = nullptr;
	ChannelReactor::onDisconnect(channel);
}

void TCPClientReactor::on_connect(uv_connect_t* req, int status)
{
	auto reactor = (TCPClientReactor*)req->handle->loop->data;

	if (status < 0)
	{
		TError("failed to connect: %s", ::uv_strerror(status));
		return;
	}

	// Get the actual ip and port number

	sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	TRef<ISocketAddress> localAddress, remoteAddress;

	auto client = (uv_tcp_t*)req->handle;
	auto result = uv_tcp_getsockname((uv_tcp_t*)client, (struct sockaddr*)&addr, &addrlen);
	if (result == 0)
	{
		if (addr.ss_family == AF_INET)
		{
			auto in_addr = (struct sockaddr_in*)&addr;
			char ip_str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
			auto address = ip_str;
			auto portNum = ntohs(in_addr->sin_port);
			localAddress = TNew<IPv4Address>(address, portNum);
		}
		else if (addr.ss_family == AF_INET6)
		{
			auto in6_addr = (struct sockaddr_in6*)&addr;
			char ip_str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
			auto portNum = ntohs(in6_addr->sin6_port);
			auto address = ip_str;
			localAddress = TNew<IPv6Address>(address, portNum);
		}
		else TError("unknown address family: %d", addr.ss_family);
	}
	else TError("failed to get socket name: %s", ::uv_strerror(result));

	result = uv_tcp_getpeername((uv_tcp_t*)client, (struct sockaddr*)&addr, &addrlen);
	if (result == 0)
	{
		if (addr.ss_family == AF_INET)
		{
			auto in_addr = (struct sockaddr_in*)&addr;
			char ip_str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
			auto address = ip_str;
			auto portNum = ntohs(in_addr->sin_port);
			remoteAddress = TNew<IPv4Address>(address, portNum);
		}
		else if (addr.ss_family == AF_INET6)
		{
			auto in6_addr = (struct sockaddr_in6*)&addr;
			char ip_str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
			auto address = ip_str;
			auto portNum = ntohs(in6_addr->sin6_port);
			remoteAddress = TNew<IPv6Address>(address, portNum);
		}
		else TError("unknown address family: %d", addr.ss_family);
	}
	else TError("failed to get socket name: %s", ::uv_strerror(result));

	if (localAddress == nullptr || remoteAddress == nullptr)
	{
		uv_close((uv_handle_t*)client, nullptr);
		return;
	}

	auto channel = TNew<TCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), client);
	client->data = channel.get();
	reactor->onConnect(channel);

	TDebug("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());

	// Start reading data from the server

	result = uv_read_start((uv_stream_t*)client, on_alloc, on_read);
	if (result)
	{
		TError("read start error: %s", ::uv_strerror(result));
		uv_close((uv_handle_t*)&client, nullptr);

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
	auto client = (uv_tcp_t*)stream;
	auto channel = ((Channel*)client->data)->shared_from_this();
	if (channel == nullptr) return;

	if (nread < 0)
	{
		uv_close((uv_handle_t*)stream, nullptr);
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	if (nread == 0)
	{
		uv_close((uv_handle_t*)stream, nullptr);
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	if (channel->running() == false)
	{
		uv_close((uv_handle_t*)stream, nullptr);
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	// 处理接收到的数据
	auto event = TNew<IChannelEvent>();
	event->Message = TString((char*)buf->base, nread);
	event->Channel = channel;
	reactor->onInbound(event);

	// 释放缓冲区
	free(buf->base);
}

void TCPClientReactor::on_send(uv_tcp_t* handle)
{
	auto reactor = (TCPClientReactor*)handle->loop->data;
	auto server = (uv_udp_t*)handle;

	if (reactor->m_Sending == false) return;

	TMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();

		if (event->Channel.expired()) continue;
		auto channel = TCast<TCPChannel>(event->Channel.lock());
		if (channel == nullptr) continue;
		if (channel->running() == false) reactor->onDisconnect(channel);
		if (channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto client = channel->getHandle();

		size_t sentNum = 0;
		while (sentNum < event->Message.size())
		{
			auto buf = uv_buf_init(event->Message.data() + sentNum, (unsigned)(event->Message.size() - sentNum));
			auto result = uv_try_write((uv_stream_t*)client, &buf, 1);
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