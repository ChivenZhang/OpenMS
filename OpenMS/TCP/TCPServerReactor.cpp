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
#include "TCPServerReactor.h"
#include "TCPChannel.h"

TCPServerReactor::TCPServerReactor(TStringView ip, uint16_t port, uint32_t backlog, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(ip),
	m_PortNum(port),
	m_Backlog(backlog),
	m_AsyncStop(uv_async_t())
{
}

void TCPServerReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	TPromise<void> promise;
	auto future = promise.get_future();
	m_EventThread = TThread([=, &promise]() {
		uv_loop_t loop;
		uv_tcp_t server;

		uv_loop_init(&loop);
		uv_tcp_init(&loop, &server);
		uv_async_init(&loop, &m_AsyncStop, on_stop);

		// Bind and listen to the socket

		{
			TPrint("binding to %s:%d", m_Address.data(), m_PortNum);

			sockaddr_in addr;
			auto result = uv_ip4_addr(m_Address.data(), m_PortNum, &addr);
			if (result) TFatal("invalid address: %s", ::uv_strerror(result));

			result = uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
			if (result) TFatal("bind error: %s", ::uv_strerror(result));

			result = uv_listen((uv_stream_t*)&server, m_Backlog, on_connect);
			if (result) TFatal("listen error: %s", ::uv_strerror(result));
		}

		// Get the actual ip and port number

		{
			sockaddr_storage addr;
			socklen_t addrlen = sizeof(addr);
			auto result = uv_tcp_getsockname((uv_tcp_t*)&server, (struct sockaddr*)&addr, &addrlen);
			if (result == 0)
			{
				if (addr.ss_family == AF_INET)
				{
					auto in_addr = (struct sockaddr_in*)&addr;
					char ip_str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
					m_PortNum = ntohs(in_addr->sin_port);
					m_Address = ip_str;
					m_SocketAddress = TNew<IPv4Address>(m_Address, m_PortNum);
				}
				else if (addr.ss_family == AF_INET6)
				{
					auto in6_addr = (struct sockaddr_in6*)&addr;
					char ip_str[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
					m_PortNum = ntohs(in6_addr->sin6_port);
					m_Address = ip_str;
					m_SocketAddress = TNew<IPv6Address>(m_Address, m_PortNum);
				}
				else TFatal("unknown address family: %d", addr.ss_family);
			}
			else TFatal("failed to get socket name: %s", ::uv_strerror(result));
		}

		// Run the event loop

		{
			loop.data = this;
			promise.set_value();
			TPrint("listening on %s:%d", m_Address.data(), m_PortNum);

			while (m_Running == true && uv_run(&loop, UV_RUN_NOWAIT))
			{
				if (m_Sending == false) continue;

				TMutexLock lock(m_EventLock);
				m_Sending = false;

				while (m_EventQueue.size())
				{
					auto event = m_EventQueue.front();
					m_EventQueue.pop();

					if (event->Channel.expired()) continue;
					auto channel = TCast<TCPChannel>(event->Channel.lock());
					if (channel == nullptr || event->Message.empty()) continue;
					auto data_to_send = (char*)::malloc(event->Message.size());
					if (data_to_send == nullptr) continue;

					auto client = channel->getHandle();
					auto data_len = (uint32_t)event->Message.size();
					::memcpy(data_to_send, event->Message.data(), data_len);
					auto write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
					if (write_req) write_req->data = data_to_send;
					uv_buf_t buf = uv_buf_init(data_to_send, data_len);
					auto result = uv_write(write_req, (uv_stream_t*)client, &buf, 1, on_write);
					if (result)
					{
						free(data_to_send);
						free(write_req);
						TError("write error: %s", ::uv_strerror(result));
					}
				}
			}
		}

		TPrint("closing server");

		uv_close((uv_handle_t*)&m_AsyncStop, nullptr);
		uv_close((uv_handle_t*)&server, nullptr);
		uv_loop_close(&loop);
		});

	future.wait();
}

void TCPServerReactor::shutdown()
{
	if (m_Running == false) return;
	uv_async_send(&m_AsyncStop);
	ChannelReactor::shutdown();
}

void TCPServerReactor::onConnect(TRef<Channel> channel)
{
	ChannelReactor::onConnect(channel);
	auto hashName = THash(channel->getRemote()->getAddress());
	m_Connections[hashName] = channel;
}

void TCPServerReactor::onDisconnect(TRef<Channel> channel)
{
	ChannelReactor::onDisconnect(channel);
	auto hashName = THash(channel->getRemote()->getAddress());
	m_Connections.erase(hashName);
}

// 处理新的连接
void TCPServerReactor::on_connect(uv_stream_t* server, int status)
{
	auto reactor = (TCPServerReactor*)server->loop->data;

	if (status < 0)
	{
		TError("new connection error: %s", uv_strerror(status));
		return;
	}

	auto client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	uv_tcp_init(server->loop, client);

	if (uv_accept(server, (uv_stream_t*)client) == 0)
	{
		TString address;
		uint16_t portNum = 0;
		TRef<IChannelSocketAddress> socketAddress;
		{
			sockaddr_storage addr;
			socklen_t addrlen = sizeof(addr);
			auto result = uv_tcp_getsockname((uv_tcp_t*)&server, (struct sockaddr*)&addr, &addrlen);
			if (result == 0)
			{
				if (addr.ss_family == AF_INET)
				{
					auto in_addr = (struct sockaddr_in*)&addr;
					char ip_str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &in_addr->sin_addr, ip_str, sizeof(ip_str));
					portNum = ntohs(in_addr->sin_port);
					address = ip_str;
					socketAddress = TNew<IPv4Address>(address, portNum);
				}
				else if (addr.ss_family == AF_INET6)
				{
					auto in6_addr = (struct sockaddr_in6*)&addr;
					char ip_str[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, &in6_addr->sin6_addr, ip_str, sizeof(ip_str));
					portNum = ntohs(in6_addr->sin6_port);
					address = ip_str;
					socketAddress = TNew<IPv6Address>(address, portNum);
				}
				else TError("unknown address family: %d", addr.ss_family);
			}
			else TError("failed to get socket name: %s", ::uv_strerror(result));
		}
		if (socketAddress == nullptr)
		{
			uv_close((uv_handle_t*)client, nullptr);
			free(client);
		}

		auto channel = TNew<TCPChannel>(reactor, reactor->m_SocketAddress, socketAddress, client);
		client->data = channel.get();
		reactor->onConnect(channel);

		auto result = uv_read_start((uv_stream_t*)client, on_alloc, on_read);
		if (result)
		{
			reactor->onDisconnect(channel->shared_from_this());

			TError("read start error: %s", ::uv_strerror(result));
		}
	}
	else
	{
		uv_close((uv_handle_t*)client, nullptr);
		free(client);
	}
}

// 申请缓冲区
void TCPServerReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

// 读取客户端数据
void TCPServerReactor::on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	auto reactor = (TCPServerReactor*)stream->loop->data;
	auto client = (uv_tcp_t*)stream;
	auto channel = (Channel*)client->data;

	if (nread < 0)
	{
		reactor->onDisconnect(channel->shared_from_this());

		uv_close((uv_handle_t*)stream, nullptr);
		free(buf->base);
		return;
	}

	if (nread == 0)
	{
		reactor->onDisconnect(channel->shared_from_this());

		uv_close((uv_handle_t*)stream, nullptr);
		free(buf->base);
		return;
	}

	// 处理接收到的数据
	auto event = TNew<IChannelEvent>();
	event->Message = TString((char*)buf->base, nread);
	event->Channel = channel->weak_from_this();
	reactor->onInbound(event);

	// 发送回显数据给客户端
	uv_write_t req;
	uv_write(&req, stream, buf, 1, nullptr);

	// 释放缓冲区
	free(buf->base);
}

void TCPServerReactor::on_write(uv_write_t* req, int status)
{
	if (status)
	{
		TError("write error: %s", uv_strerror(status));
	}
	else
	{
		TDebug("data sent successfully");
	}
	free((void*)req->data);
	free(req);
}

// 停止事件循环
void TCPServerReactor::on_stop(uv_async_t* handle)
{
	uv_stop(handle->loop);
}