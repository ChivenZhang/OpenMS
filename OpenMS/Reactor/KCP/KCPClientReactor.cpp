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
#include "KCPClientReactor.h"
#include "KCPChannel.h"
#include "../../External/kcp/ikcp.h"

#if 1 // From official kcp demo: https://github.com/skywind3000/kcp/blob/master/test.h

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#elif !defined(__unix)
#define __unix
#endif

#ifdef __unix
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#endif

/* get system time */
static inline void itimeofday(long* sec, long* usec)
{
#if defined(__unix)
	struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;
#else
	static long mode = 0, addsec = 0;
	BOOL retval;
	static IINT64 freq = 1;
	IINT64 qpc;
	if (mode == 0) {
		retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		freq = (freq == 0) ? 1 : freq;
		retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
		addsec = (long)time(NULL);
		addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
		mode = 1;
	}
	retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
	retval = retval * 2;
	if (sec) *sec = (long)(qpc / freq) + addsec;
	if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
#endif
}

/* get clock in millisecond 64 */
static inline IINT64 iclock64(void)
{
	long s, u;
	IINT64 value;
	itimeofday(&s, &u);
	value = ((IINT64)s) * 1000 + (u / 1000);
	return value;
}

static inline IUINT32 iclock()
{
	return (IUINT32)(iclock64() & 0xfffffffful);
}

#endif

KCPClientReactor::KCPClientReactor(TRef<ISocketAddress> address, size_t workerNum, callback_kcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(address)
{
	if (m_Address == nullptr) m_Address = TNew<IPv4Address>("0.0.0.0", 0);
}

void KCPClientReactor::startup()
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

				result = uv_udp_recv_start(&client, on_alloc, on_read);
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
				m_LocalAddress = localAddress;

				TPrint("listening on %s:%d", localAddress->getAddress().c_str(), localAddress->getPort());
			}

			// Send the initial data for session id

			{
				size_t sentNum = 0;
				TStringView message("\0", 1);
				while (sentNum < message.size())
				{
					auto buf = uv_buf_init((char*)message.data() + sentNum, (unsigned)(message.size() - sentNum));
					auto result = uv_udp_try_send(&client, &buf, 1, nullptr);
					if (result < 0) break;
					else if (result == UV_EAGAIN) continue;
					else sentNum += result;
				}
			}

			// Run the event loop

			{
				loop.data = this;
				m_Connect = true;
				promise.set_value();

				while (m_Running == true && uv_run(&loop, UV_RUN_NOWAIT))
				{
					on_send(&client);
					if (m_ChannelRemoved) m_ChannelRemoved = nullptr;
				}
				m_Connect = false;
			}

			// Close all channels

			{
				if (m_Channel) onOnClose(m_Channel);
				m_Channel = nullptr;
				m_ChannelRemoved = nullptr;
			}

		} while (0);

		uv_close((uv_handle_t*)&client, nullptr);
		uv_loop_close(&loop);

		TPrint("closed client");
		});

	future.wait();
}

void KCPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_Channel = nullptr;
	m_ChannelRemoved = nullptr;
}

THnd<IChannelAddress> KCPClientReactor::address() const
{
	return m_Connect ? m_LocalAddress : THnd<IChannelAddress>();
}

void KCPClientReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->write(event);
}

void KCPClientReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->writeAndFlush(event);
}

void KCPClientReactor::onConnect(TRef<Channel> channel)
{
	m_Channel = channel;
	ChannelReactor::onConnect(channel);
}

void KCPClientReactor::onOnClose(TRef<Channel> channel)
{
	m_Channel = nullptr;
	m_ChannelRemoved = channel;
	ChannelReactor::onOnClose(channel);
}

void KCPClientReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void KCPClientReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* peer, unsigned flags)
{
	auto reactor = (KCPClientReactor*)req->loop->data;
	auto channel = reactor->m_Channel;
	auto client = (uv_udp_t*)req;

	if (nread == 0 || peer == nullptr) return;

	if (channel == nullptr)
	{
		// Get the actual ip and port number

		sockaddr_storage addr;
		socklen_t addrlen = sizeof(addr);
		TRef<ISocketAddress> localAddress, remoteAddress;

		auto result = uv_udp_getsockname((uv_udp_t*)client, (sockaddr*)&addr, &addrlen);
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

		result = uv_udp_getpeername((uv_udp_t*)client, (sockaddr*)&addr, &addrlen);
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

		if (localAddress == nullptr || remoteAddress == nullptr)
		{
			free(buf->base);
			return;
		}

		// Create a new channel and session

		auto sessionID = ikcp_getconv(buf->base);
		auto session = ikcp_create(sessionID, nullptr);
		channel = TNew<KCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), client, session);
		session->user = channel.get();
		ikcp_setoutput(session, on_output);
		ikcp_wndsize(session, 128, 128);
		ikcp_nodelay(session, 1, 20, 2, 1);
		reactor->onConnect(channel);

		TDebug("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());

		free(buf->base);
		return;
	}

	// Process the incoming data : nread >= 0

	if (nread < 0)
	{
		free(buf->base);

		reactor->onOnClose(channel);
		return;
	}

	if (channel->running() == false)
	{
		free(buf->base);

		reactor->onOnClose(channel);
		return;
	}

	auto _channel = TCast<KCPChannel>(channel);
	auto result = ikcp_input(_channel->getSession(), (char*)buf->base, (uint32_t)nread);
	if (result < 0)
	{
		free(buf->base);

		reactor->onOnClose(channel);
		return;
	}

	free(buf->base);
}

int KCPClientReactor::on_output(const char* buf, int len, IKCPCB* kcp, void* user)
{
	auto channel = (KCPChannel*)user;
	auto reactor = TCast<KCPClientReactor>(channel->getReactor());
	auto client = (uv_udp_t*)channel->getHandle();
	auto remote = channel->getRemote().lock();

	// Send the data

	size_t sentNum = 0;
	TStringView message(buf, len);
	while (sentNum < message.size())
	{
		auto buf = uv_buf_init((char*)message.data() + sentNum, (unsigned)(message.size() - sentNum));
		auto result = uv_udp_try_send(client, &buf, 1, nullptr);
		if (result < 0)
		{
			reactor->onOnClose(channel->shared_from_this());
			return -1;
		}
		else if (result == UV_EAGAIN) continue;
		else sentNum += result;
	}
	return (uint32_t)sentNum;
}

void KCPClientReactor::on_send(uv_udp_t* handle)
{
	auto reactor = (KCPClientReactor*)handle->loop->data;
	auto client = (uv_udp_t*)handle;

	if (reactor->m_Channel)
	{
		auto channel = TCast<KCPChannel>(reactor->m_Channel);
		if (channel == nullptr || channel->running() == false) return;
		auto session = channel->getSession();

		auto nowTime = iclock();
		auto nextTime = ikcp_check(session, nowTime);
		if (nextTime <= nowTime) ikcp_update(session, nowTime);

		char buffer[2048];
		auto result = ikcp_recv(channel->getSession(), buffer, sizeof(buffer));
		if (0 < result)
		{
			auto event = TNew<IChannelEvent>();
			event->Message = TStringView(buffer, result);
			event->Channel = channel;
			while (0 <= result)
			{
				result = ikcp_recv(channel->getSession(), buffer, sizeof(buffer));
				if (result < 0) break;
				event->Message += TStringView(buffer, result);
			}
			reactor->onInbound(event);
		}
	}

	if (reactor->m_Sending == false) return;

	TMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();

		auto channel = TCast<KCPChannel>(event->Channel.lock());
		if (channel == nullptr) continue;
		if (channel->running() == false) reactor->onOnClose(channel);
		if (channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto session = channel->getSession();

		size_t sentNum = 0;
		while (sentNum < event->Message.size())
		{
			auto buf = uv_buf_init(event->Message.data() + sentNum, (unsigned)(event->Message.size() - sentNum));
			auto result = ikcp_send(session, (char*)buf.base, buf.len);
			if (result < 0)
			{
				reactor->onOnClose(channel);
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