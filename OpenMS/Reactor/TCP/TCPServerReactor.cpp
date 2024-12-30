#include "TCPServerReactor.h"
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "TCPServerReactor.h"
#include "TCPChannel.h"

TCPServerReactor::TCPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_tcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Backlog(backlog ? backlog : 128),
	m_Address(address)
{
	if (m_Address == nullptr) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void TCPServerReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = MSThread([=, &promise]() {
		uv_loop_t loop;
		uv_tcp_t server;

		uv_loop_init(&loop);
		uv_tcp_init(&loop, &server);
		uv_tcp_nodelay(&server, 1);

		do
		{
			// Bind and listen to the socket

			{
				sockaddr_storage addr;
				uint32_t result = uv_errno_t::UV_ERRNO_MAX;
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

				result = uv_tcp_bind(&server, (const sockaddr*)&addr, 0);
				if (result) MS_ERROR("bind error: %s", ::uv_strerror(result));
				if (result) break;

				result = uv_listen((uv_stream_t*)&server, m_Backlog, on_connect);
				if (result) MS_ERROR("listen error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			{
				sockaddr_storage addr;
				int addrlen = sizeof(addr);
				MSRef<ISocketAddress> localAddress;

				auto result = uv_tcp_getsockname((uv_tcp_t*)&server, (sockaddr*)&addr, &addrlen);
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

			// Run the event loop

			{
				loop.data = this;
				m_Connect = true;
				promise.set_value();

				while (m_Running == true && uv_run(&loop, UV_RUN_NOWAIT))
				{
					on_send(&server);
				}
				m_Connect = false;
			}

			// Close all channels

			{
				auto channels = m_ChannelMap;
				for (auto channel : channels)
				{
					if (channel.second) onDisconnect(channel.second);
				}
				m_ChannelMap.clear();
			}

			uv_close((uv_handle_t*)&server, nullptr);
			uv_loop_close(&loop);

			MS_PRINT("closed server");
			return;
		} while (0);

		uv_close((uv_handle_t*)&server, nullptr);
		uv_loop_close(&loop);
		promise.set_value();
		});

	future.wait();
}

void TCPServerReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();

	m_ChannelMap.clear();
}

MSHnd<IChannelAddress> TCPServerReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void TCPServerReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelMap[hashName] = channel;
	ChannelReactor::onConnect(channel);
}

void TCPServerReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	auto remote = MSCast<ISocketAddress>(channel->getRemote().lock());
	auto hashName = remote->getHashName();
	m_ChannelMap.erase(hashName);
	ChannelReactor::onDisconnect(channel);
}

void TCPServerReactor::on_connect(uv_stream_t* server, int status)
{
	auto reactor = (TCPServerReactor*)server->loop->data;

	if (status < 0)
	{
		MS_ERROR("new connection error: %s", uv_strerror(status));
		return;
	}

	auto client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	uv_tcp_init(server->loop, client);

	if (uv_accept(server, (uv_stream_t*)client) == 0)
	{
		// Get the actual ip and port number

		sockaddr_storage addr;
		int addrlen = sizeof(addr);
		MSRef<ISocketAddress> localAddress, remoteAddress;

		auto result = uv_tcp_getsockname((uv_tcp_t*)client, (sockaddr*)&addr, &addrlen);
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

		result = uv_tcp_getpeername((uv_tcp_t*)client, (sockaddr*)&addr, &addrlen);
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
				auto address = ip_str;
				auto portNum = ntohs(in6_addr->sin6_port);
				remoteAddress = MSNew<IPv6Address>(address, portNum);
			}
			else MS_ERROR("unknown address family: %d", addr.ss_family);
		}
		else MS_ERROR("failed to get socket name: %s", ::uv_strerror(result));

		if (localAddress == nullptr || remoteAddress == nullptr)
		{
			uv_close((uv_handle_t*)client, nullptr);
			free(client);
			return;
		}

		auto channel = MSNew<TCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), client);
		client->data = channel.get();
		reactor->onConnect(channel);

		// Start reading data from the client

		result = uv_read_start((uv_stream_t*)client, on_alloc, on_read);
		if (result)
		{
			MS_ERROR("readChannel start error: %s", ::uv_strerror(result));
			uv_close((uv_handle_t*)&client, nullptr);
			free(client);

			reactor->onDisconnect(channel->shared_from_this());
			return;
		}
	}
	else
	{
		uv_close((uv_handle_t*)client, nullptr);
		free(client);
	}
}

void TCPServerReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void TCPServerReactor::on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	auto reactor = (TCPServerReactor*)stream->loop->data;
	auto client = (uv_tcp_t*)stream;
	auto channel = ((TCPChannel*)client->data)->shared_from_this();
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

	auto event = MSNew<IChannelEvent>();
	event->Message = MSString((char*)buf->base, nread);
	event->Channel = channel;
	reactor->onInbound(event);

	free(buf->base);
}

void TCPServerReactor::on_send(uv_tcp_t* handle)
{
	auto reactor = (TCPServerReactor*)handle->loop->data;
	if (reactor->m_Sending == false) return;

	MSMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();

		auto channel = MSCast<TCPChannel>(event->Channel.lock());
		if (channel == nullptr) continue;
		if (channel->running() == false) reactor->onDisconnect(channel);
		if (channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto client = channel->getHandle();

		size_t i = 0;
		while(i < event->Message.size())
		{
			auto buf = uv_buf_init(event->Message.data() + i, (unsigned)(event->Message.size() - i));
			auto result = uv_try_write((uv_stream_t*)client, &buf, 1);
			if (result == UV_EAGAIN) continue;
			else if (result < 0) break;
			else i += result;
		}
		if(i != event->Message.size()) reactor->onDisconnect(channel);
		if (event->Promise) event->Promise->set_value(i == event->Message.size());
	}
}