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
#include "KCPServerReactor.h"
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

KCPServerReactor::KCPServerReactor(TRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_kcp_t callback)
	:
	ChannelReactor(workerNum, { callback.Connected, callback.Disconnect }),
	m_Backlog(backlog ? backlog : 128),
	m_Session(1),
	m_Address(address),
	m_OnSession(callback.Session),
	m_AsyncStop(uv_async_t())
{
	if (m_Address == nullptr) m_Address = TNew<IPv4Address>("0.0.0.0", 0);
	if (m_OnSession == nullptr) m_OnSession = [=](TRef<IChannelAddress>) { return m_Session++; };
}

void KCPServerReactor::startup()
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

				result = uv_udp_bind(&server, (sockaddr*)&addr, 0);
				if (result) TError("bind error: %s", ::uv_strerror(result));
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

			// Start receiving data

			{
				auto result = uv_udp_recv_start(&server, on_alloc, on_read);
				if (result) TError("recv start error: %s", ::uv_strerror(result));
				if (result) break;
			}

			// Run the event loop

			{
				loop.data = this;
				promise.set_value();

				while (m_Running == true && uv_run(&loop, UV_RUN_NOWAIT))
				{
					on_send(&server);
					if (m_ChannelsRemoved.size()) m_ChannelsRemoved.clear();
				}
			}

		} while (0);

		TPrint("closing server");

		uv_close((uv_handle_t*)&m_AsyncStop, nullptr);
		uv_close((uv_handle_t*)&server, nullptr);
		uv_loop_close(&loop);
		});

	future.wait();
}

void KCPServerReactor::shutdown()
{
	if (m_Running == false) return;
	uv_async_send(&m_AsyncStop);
	ChannelReactor::shutdown();
	m_Channels.clear();
	m_ChannelMap.clear();
	m_ChannelsRemoved.clear();
}

void KCPServerReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto remote = TCast<ISocketAddress>(address);
	if (remote == nullptr || event == nullptr) return;
	auto hashName = remote->getHashName();
	auto result = m_ChannelMap.find(hashName);
	if (result == m_ChannelMap.end() && result->second == nullptr) return;
	auto channel = result->second;
	event->Channel = channel;
	if (channel && channel->running()) channel->write(event);
}

void KCPServerReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
	if (m_Running == false) return;
	auto remote = TCast<ISocketAddress>(address);
	if (remote == nullptr || event == nullptr) return;
	auto hashName = remote->getHashName();
	auto result = m_ChannelMap.find(hashName);
	if (result == m_ChannelMap.end() && result->second == nullptr) return;
	auto channel = result->second;
	event->Channel = channel;
	if (channel && channel->running()) channel->writeAndFlush(event);
}

void KCPServerReactor::onConnect(TRef<Channel> channel)
{
	auto remote = TCast<ISocketAddress>(channel->getRemote());
	auto hashName = remote->getHashName();
	m_Channels.insert(m_Channels.begin(), channel);
	m_ChannelMap[hashName] = channel;
	if (m_Backlog < m_Channels.size()) onDisconnect(m_Channels.back());
	ChannelReactor::onConnect(channel);
}

void KCPServerReactor::onDisconnect(TRef<Channel> channel)
{
	auto remote = TCast<ISocketAddress>(channel->getRemote());
	auto hashName = remote->getHashName();
	m_ChannelsRemoved.push_back(channel);
	m_ChannelMap.erase(hashName);
	m_Channels.erase(std::remove(m_Channels.begin(), m_Channels.end(), channel), m_Channels.end());
	ChannelReactor::onDisconnect(channel);
}

void KCPServerReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = (uint32_t)suggested_size;
}

void KCPServerReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* peer, unsigned flags)
{
	auto reactor = (KCPServerReactor*)req->loop->data;
	auto server = (uv_udp_t*)req;

	if (nread == 0 || peer == nullptr) return;

	// Get the remote address hash

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

	auto channel = reactor->m_ChannelMap[hashName];
	if (channel == nullptr)
	{
		// Get the actual ip and port number

		sockaddr_storage addr;
		socklen_t addrlen = sizeof(addr);
		TRef<ISocketAddress> localAddress, remoteAddress;

		auto result = uv_udp_getsockname((uv_udp_t*)server, (sockaddr*)&addr, &addrlen);
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

		// Create a new channel and session

		auto sessionID = reactor->m_OnSession(remoteAddress);
		auto session = ikcp_create(sessionID, nullptr);
		channel = TNew<KCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), server, session);
		session->user = channel.get();
		ikcp_setoutput(session, on_output);
		ikcp_wndsize(session, 128, 128);
		ikcp_nodelay(session, 1, 20, 2, 1);
		// Pass the session id to remote client
		result = ikcp_send(session, nullptr, 0);
		if (result < 0) free(buf->base);
		if (result < 0)return;
		reactor->onConnect(channel);

		TDebug("accepted from %s:%d", remoteAddress->getAddress().c_str(), remoteAddress->getPort());

		free(buf->base);
		return;
	}

	// Process the incoming data : nread >= 0

	if (nread < 0)
	{
		reactor->onDisconnect(channel);

		free(buf->base);
		return;
	}

	auto _channel = TCast<KCPChannel>(channel);
	auto result = ikcp_input(_channel->getSession(), (char*)buf->base, (uint32_t)nread);
	if (result < 0)
	{
		reactor->onDisconnect(channel);

		free(buf->base);
		return;
	}

	free(buf->base);
}

int KCPServerReactor::on_output(const char* buf, int len, IKCPCB* kcp, void* user)
{
	auto channel = (KCPChannel*)user;
	auto reactor = TCast<KCPServerReactor>(channel->getReactor());
	auto server = (uv_udp_t*)channel->getHandle();
	auto remote = channel->getRemote();

	sockaddr_storage addr;
	int result = uv_errno_t::UV_EINVAL;
	if (auto ipv4 = TCast<IPv4Address>(remote))
	{
		result = uv_ip4_addr(ipv4->getAddress().c_str(), ipv4->getPort(), (sockaddr_in*)&addr);
	}
	else if (auto ipv6 = TCast<IPv6Address>(remote))
	{
		result = uv_ip6_addr(ipv6->getAddress().c_str(), ipv6->getPort(), (sockaddr_in6*)&addr);
	}
	if (result) TError("invalid address: %s", ::uv_strerror(result));
	if (result) return -1;

	size_t sentNum = 0;
	TStringView message(buf, len);
	while (sentNum < message.size())
	{
		auto buf = uv_buf_init((char*)message.data() + sentNum, (unsigned)(message.size() - sentNum));
		auto result = uv_udp_try_send(server, &buf, 1, (sockaddr*)&addr);
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

void KCPServerReactor::on_send(uv_udp_t* handle)
{
	auto reactor = (KCPServerReactor*)handle->loop->data;

	// Update all kcp sessions

	for (size_t i = 0; i < reactor->m_Channels.size(); ++i)
	{
		auto channel = TCast<KCPChannel>(reactor->m_Channels[i]);
		if (channel == nullptr || channel->running() == false) continue;
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
			event->Channel = channel->weak_from_this();
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

	// Send all pending data

	TMutexLock lock(reactor->m_EventLock);
	reactor->m_Sending = false;

	while (reactor->m_EventQueue.size())
	{
		auto event = reactor->m_EventQueue.front();
		reactor->m_EventQueue.pop();

		auto channel = TCast<KCPChannel>(event->Channel.lock());
		if (channel == nullptr || channel->running() == false) continue;
		if (event->Message.empty()) continue;
		auto server = channel->getHandle();
		auto session = channel->getSession();
		auto remote = channel->getRemote();

		size_t sentNum = 0;
		while (sentNum < event->Message.size())
		{
			auto buf = uv_buf_init(event->Message.data() + sentNum, (unsigned)(event->Message.size() - sentNum));
			auto result = ikcp_send(session, (char*)buf.base, buf.len);
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

void KCPServerReactor::on_stop(uv_async_t* handle)
{
	uv_stop(handle->loop);
}