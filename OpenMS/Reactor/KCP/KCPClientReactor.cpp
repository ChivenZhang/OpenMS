/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "KCPClientReactor.h"
#include "KCPChannel.h"
#include <ikcp.h>

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

KCPClientReactor::KCPClientReactor(MSRef<ISocketAddress> address, size_t workerNum, callback_kcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(address)
{
	if (m_Address == nullptr) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void KCPClientReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = MSThread([=, &promise]() {
		uv_loop_t loop;
		uv_udp_t client;

		uv_loop_init(&loop);
		uv_udp_init(&loop, &client);

		do
		{
			// Bind and listen to the socket

			if(true)
			{
				sockaddr_storage addr = {};
				uint32_t result = uv_errno_t::UV_EINVAL;
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

				result = uv_udp_recv_start(&client, on_alloc, on_read);
				if (result) MS_ERROR("recv start error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Get the actual ip and port number

			if (true)
			{
				sockaddr_storage addr = {};
				int addrLen = sizeof(addr);
				MSRef<ISocketAddress> localAddress;

				auto result = uv_udp_getsockname(&client, reinterpret_cast<sockaddr *>(&addr), &addrLen);
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

			// Send the initial data for session id

			if (true)
			{
				size_t sentNum = 0;
				MSStringView message("\0", 1);
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

			if (true)
			{
				loop.data = this;
				m_Connect = true;
				promise.set_value();
				while (m_Running && uv_run(&loop, UV_RUN_NOWAIT))
				{
					on_send(&client);
					if (m_ChannelRemoved) m_ChannelRemoved = nullptr;
				}
				m_Connect = false;
			}

			// Close all channels

			if (true)
			{
				if (m_Channel) onDisconnect(m_Channel);
				m_Channel = nullptr;
				m_ChannelRemoved = nullptr;
			}

			uv_close((uv_handle_t*)&client, nullptr);
			uv_loop_close(&loop);

			MS_PRINT("closed client");
			return;
		} while (0);

		uv_close((uv_handle_t*)&client, nullptr);
		uv_loop_close(&loop);
		promise.set_value();
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

MSHnd<IChannelAddress> KCPClientReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void KCPClientReactor::write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->write(event);
}

void KCPClientReactor::writeAndFlush(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->writeAndFlush(event);
}

void KCPClientReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	m_Channel = channel;
	ChannelReactor::onConnect(channel);
}

void KCPClientReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	m_Channel = nullptr;
	m_ChannelRemoved = channel;
	ChannelReactor::onDisconnect(channel);
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
	auto client = req;

	if (nread == 0 || peer == nullptr) return;

	if (channel == nullptr)
	{
		// Get the actual ip and port number

		sockaddr_storage addr = {};
		int addrLen = sizeof(addr);
		MSRef<ISocketAddress> localAddress, remoteAddress;

		auto result = uv_udp_getsockname(client, (sockaddr*)&addr, &addrLen);
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

		result = uv_udp_getpeername(client, (sockaddr*)&addr, &addrLen);
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

		if (localAddress == nullptr || remoteAddress == nullptr)
		{
			free(buf->base);
			return;
		}

		// Create a new channel and session

		auto sessionID = ikcp_getconv(buf->base);
		auto session = ikcp_create(sessionID, nullptr);
		channel = MSNew<KCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), client, session);
		session->user = channel.get();
		ikcp_setoutput(session, on_output);
		ikcp_wndsize(session, 128, 128);
		ikcp_nodelay(session, 1, 20, 2, 1);
		reactor->onConnect(channel);

		MS_DEBUG("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());

		free(buf->base);
		return;
	}

	// Process the incoming data : nread >= 0

	if (nread < 0)
	{
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	if (channel->running() == false)
	{
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	auto _channel = MSCast<KCPChannel>(channel);
	auto result = ikcp_input(_channel->getSession(), (char*)buf->base, (uint32_t)nread);
	if (result < 0)
	{
		free(buf->base);

		reactor->onDisconnect(channel);
		return;
	}

	free(buf->base);
}

int KCPClientReactor::on_output(const char* buf, int len, IKCPCB* kcp, void* user)
{
	auto channel = (KCPChannel*)user;
	auto reactor = MSCast<KCPClientReactor>(channel->getReactor());
	auto client = (uv_udp_t*)channel->getHandle();
	auto remote = channel->getRemote().lock();

	// Send the data

	size_t sentNum = 0;
	MSStringView message(buf, len);
	while (sentNum < message.size())
	{
		auto buf = uv_buf_init((char*)message.data() + sentNum, (unsigned)(message.size() - sentNum));
		auto result = uv_udp_try_send(client, &buf, 1, nullptr);
		if (result < 0)
		{
			reactor->onDisconnect(channel->shared_from_this());
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
		auto channel = MSCast<KCPChannel>(reactor->m_Channel);
		if (channel == nullptr || channel->running() == false) return;
		auto session = channel->getSession();

		auto nowTime = iclock();
		auto nextTime = ikcp_check(session, nowTime);
		if (nextTime <= nowTime) ikcp_update(session, nowTime);

		char buffer[2048];
		auto result = ikcp_recv(channel->getSession(), buffer, sizeof(buffer));
		if (0 < result)
		{
			auto event = MSNew<IChannelEvent>();
			event->Message = MSStringView(buffer, result);
			event->Channel = channel;
			while (0 <= result)
			{
				result = ikcp_recv(channel->getSession(), buffer, sizeof(buffer));
				if (result < 0) break;
				event->Message += MSStringView(buffer, result);
			}
			reactor->onInbound(event);
		}
	}

	if (reactor->m_Sending == false) return;

	MSMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();

		auto channel = MSCast<KCPChannel>(event->Channel.lock());
		if (channel == nullptr) continue;
		if (channel->running() == false) reactor->onDisconnect(channel);
		if (channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto session = channel->getSession();

		size_t i = 0;
		while (i < event->Message.size())
		{
			auto buf = uv_buf_init(event->Message.data() + i, (unsigned)(event->Message.size() - i));
			auto result = ikcp_send(session, (char*)buf.base, buf.len);
			if (result < 0) break;
			else i += result;
		}
		if (i != event->Message.size()) reactor->onDisconnect(channel);
		if (event->Promise) event->Promise->set_value(i == event->Message.size());
	}
}