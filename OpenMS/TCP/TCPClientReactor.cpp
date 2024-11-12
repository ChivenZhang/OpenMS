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
#include "TCPClientReactor.h"
#include "TCPChannel.h"

TCPClientReactor::TCPClientReactor(TRef<ISocketAddress> address, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_SocketAddress(address),
	m_AsyncStop(uv_async_t())
{
	if (m_SocketAddress == nullptr) m_SocketAddress = TNew<IPv4Address>("0.0.0.0", 0);
	m_Address = m_SocketAddress->getAddress();
	m_PortNum = m_SocketAddress->getPort();
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
		uv_async_init(&loop, &m_AsyncStop, on_stop);

		do
		{
			// Set up the client

			{
				sockaddr_storage addr;
				uint32_t result = uv_errno_t::UV_ERRNO_MAX;
				if (TCast<IPv4Address>(m_SocketAddress))
				{
					result = uv_ip4_addr(m_Address.data(), m_PortNum, (sockaddr_in*)&addr);
				}
				else if (TCast<IPv6Address>(m_SocketAddress))
				{
					result = uv_ip6_addr(m_Address.data(), m_PortNum, (sockaddr_in6*)&addr);
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
						auto portNum = ntohs(in6_addr->sin6_port);
						auto address = ip_str;
						localAddress = TNew<IPv6Address>(address, portNum);
					}
					else TError("unknown address family: %d", addr.ss_family);
				}
				else TError("failed to get socket name: %s", ::uv_strerror(result));

				if (localAddress == nullptr)
				{
					uv_close((uv_handle_t*)&client, nullptr);
					break;
				}

				TPrint("listening on %s:%d", localAddress->getAddress().c_str(), localAddress->getPort());
			}

			// Run the event loop

			{
				loop.data = this;
				promise.set_value();

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
						if (channel == nullptr || channel->running() == false) continue;
						if (event->Message.empty()) continue;
						auto client = channel->getHandle();

						size_t sentNum = 0;
						while (sentNum < event->Message.size())
						{
							auto buf = uv_buf_init(event->Message.data() + sentNum, (unsigned)(event->Message.size() - sentNum));
							auto result = uv_try_write((uv_stream_t*)client, &buf, 1);
							if (result < 0)
							{
								this->onDisconnect(channel);
								break;
							}
							else if (result == UV_EAGAIN) continue;
							else sentNum += result;
						}
						if (event->UserData)
						{
							auto promise = (TPromise<bool>*)event->UserData;
							if (promise) promise->set_value(sentNum == event->Message.size());
						}
					}
				}
			}

		} while (0);

		TPrint("closing client");

		uv_close((uv_handle_t*)&m_AsyncStop, nullptr);
		uv_close((uv_handle_t*)&client, nullptr);
		uv_loop_close(&loop);
		});

	future.wait();
}

void TCPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
}

void TCPClientReactor::onConnect(TRef<Channel> channel)
{
	ChannelReactor::onConnect(channel);
	m_Channel = channel;
}

void TCPClientReactor::onDisconnect(TRef<Channel> channel)
{
	ChannelReactor::onDisconnect(channel);
	m_Channel = nullptr;
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

	auto channel = TNew<TCPChannel>(reactor, localAddress, remoteAddress, client);
	client->data = channel.get();
	reactor->onConnect(channel);

	TDebug("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());

	// Start reading data from the server

	result = uv_read_start((uv_stream_t*)client, on_alloc, on_read);
	if (result)
	{
		reactor->onDisconnect(channel->shared_from_this());

		TError("read start error: %s", ::uv_strerror(result));
		uv_close((uv_handle_t*)&client, nullptr);
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

	// 释放缓冲区
	free(buf->base);
}

void TCPClientReactor::on_stop(uv_async_t* handle)
{
	uv_stop(handle->loop);
}