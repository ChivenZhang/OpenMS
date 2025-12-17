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
#include "KCPClientReactor.h"
#include "KCPChannel.h"

// From official kcp demo: https://github.com/skywind3000/kcp/blob/master/test.h
namespace
{
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
	void itimeofday(long* sec, long* usec)
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
	IINT64 iclock64(void)
	{
		long s, u;
		IINT64 value;
		itimeofday(&s, &u);
		value = ((IINT64)s) * 1000 + (u / 1000);
		return value;
	}

	IUINT32 iclock()
	{
		return (IUINT32)(iclock64() & 0xfffffffful);
	}
}

KCPClientReactor::KCPClientReactor(MSRef<ISocketAddress> address, size_t workerNum, callback_kcp_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<IPv4Address>("0.0.0.0", 0);
}

void KCPClientReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = MSThread([=, &promise]()
	{
		asio::io_context loop;
		using namespace asio::ip;

		do
		{
			// Bind and listen to the socket

			auto reactor = this;
			udp::socket client(loop);
			udp::endpoint endpoint(address::from_string(m_Address->getAddress()), m_Address->getPort());
			asio::error_code error;
			error = client.open(endpoint.protocol(), error);
			if (error)
			{
				MS_ERROR("failed to open: %s", error.message().c_str());
				break;
			}
			error = client.connect(endpoint, error);
			if (error)
			{
				MS_ERROR("failed to connect: %s", error.message().c_str());
				break;
			}

			// Get the actual ip and port number

			MSRef<ISocketAddress> localAddress, remoteAddress;
			if (true)
			{
				{
					auto address = client.local_endpoint().address().to_string();
					auto portNum = client.local_endpoint().port();
					auto family = client.local_endpoint().protocol().family();
					if (family == AF_INET) localAddress = MSNew<IPv4Address>(address, portNum);
					else if (family == AF_INET6) localAddress = MSNew<IPv6Address>(address, portNum);
					else MS_ERROR("unknown address family: %d", family);
				}
				{
					auto address = client.remote_endpoint().address().to_string();
					auto portNum = client.remote_endpoint().port();
					auto family = client.remote_endpoint().protocol().family();
					if (family == AF_INET) remoteAddress = MSNew<IPv4Address>(address, portNum);
					else if (family == AF_INET6) remoteAddress = MSNew<IPv6Address>(address, portNum);
					else MS_ERROR("unknown address family: %d", family);
				}
				if (localAddress == nullptr || remoteAddress == nullptr)
				{
					error = client.shutdown(tcp::socket::shutdown_both, error);
					if (error) MS_ERROR("failed to shutdown: %s", error.message().c_str());
					error = client.close(error);
					if (error) MS_ERROR("failed to close: %s", error.message().c_str());
					return;
				}
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			// Read and write data in async way

			char buffer[2048];

			MSLambda<void(MSHnd<KCPChannel> channel)> read_func;
			read_func = [&](MSHnd<KCPChannel> channel)
			{
				if (auto _channel = channel.lock())
				{
					client.async_receive(asio::buffer(buffer), [=, &read_func](asio::error_code error, size_t length)
					{
						if (error)
						{
							MS_ERROR("can't read from socket: %s", error.message().c_str());
							reactor->onDisconnect(channel.lock());
						}
						else
						{
							ikcp_input(_channel->getSession(), buffer, length);
							read_func(channel);
						}
					});
				}
			};

			// Create a new channel and session

			client.send(asio::buffer("\0", 1), 0, error);
			if (error)
			{
				MS_ERROR("failed to send: %s", error.message().c_str());
				break;
			}
			client.receive(asio::buffer(buffer), 0, error);
			if (error)
			{
				MS_ERROR("failed to receive from socket: %s", error.message().c_str());
				break;
			}
			auto session = ikcp_create(ikcp_getconv(buffer), nullptr);
			auto channel = MSNew<KCPChannel>(reactor, localAddress, remoteAddress, (uint32_t)(rand() % reactor->m_WorkerList.size()), session, &client, client.remote_endpoint());
			session->user = channel.get();
			ikcp_setoutput(session, on_output);
			ikcp_wndsize(session, 128, 128);
			ikcp_nodelay(session, 1, 20, 2, 1);
			onConnect(channel);
			read_func(MSCast<KCPChannel>(m_Channel));

			asio::steady_timer ticker(loop);
			MSLambda<void()> tick_func;
			tick_func = [&]()
			{
				ticker.async_wait([&](asio::error_code)
				{
					reactor->m_ChannelRemoved = nullptr;

					// Update all kcp sessions

					do
					{
						if (channel == nullptr || channel->running() == false) break;
						auto session = channel->getSession();

						auto nowTime = iclock();
						auto nextTime = ikcp_check(session, nowTime);
						if (nextTime <= nowTime) ikcp_update(session, nowTime);

						char buffer2[2048];
						auto length = ikcp_recv(session, buffer2, sizeof(buffer2));
						if (0 < length)
						{
							auto event = MSNew<IChannelEvent>();
							event->Message = MSStringView(buffer2, length);
							event->Channel = channel;
							reactor->onInbound(event);
						}
					} while (false);

					MSMutexLock lock(reactor->m_EventLock);
					while (reactor->m_EventQueue.size())
					{
						auto event = reactor->m_EventQueue.front();
						reactor->m_EventQueue.pop();

						auto result = -1;
						do
						{
							auto channel = MSCast<KCPChannel>(event->Channel.lock());
							if (channel == nullptr || channel->running() == false) break;
							if (event->Message.empty()) break;

							result = ikcp_send(session, event->Message.data(), (int)event->Message.size());
							if (result < 0) reactor->onDisconnect(channel);
						} while (false);

						if (event->Promise) event->Promise->set_value(result == 0);
					}

					tick_func();
				});
			};
			tick_func();

			// Run the event loop

			asio::steady_timer timer(loop);
			MSLambda<void()> timer_func;
			timer_func = [&]()
			{
				timer.expires_after(std::chrono::milliseconds(1000));
				timer.async_wait([&](asio::error_code)
				{
					if (m_Running == false) loop.stop();
					else timer_func();
				});
			};
			timer_func();
			m_Connect = true;
			promise.set_value();
			loop.run();
			m_Connect = false;

			// Close all channels

			if (true)
			{
				if (m_Channel) onDisconnect(m_Channel);
				m_Channel = nullptr;
				m_ChannelRemoved = nullptr;
			}

			MS_PRINT("closed client");
			return;

		} while (false);
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

void KCPClientReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	event->Channel = channel;
	if (channel && channel->running()) channel->write(event);
}

void KCPClientReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	m_Connect = true;
	m_Channel = channel;
	ChannelReactor::onConnect(channel);
}

void KCPClientReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	m_Connect = false;
	m_Channel = nullptr;
	m_ChannelRemoved = channel;
	ChannelReactor::onDisconnect(channel);
}

void KCPClientReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	ChannelReactor::onOutbound(event, flush);
}

int KCPClientReactor::on_output(const char* buf, int len, IKCPCB* kcp, void* user)
{
	auto channel = (KCPChannel*)user;
	auto socket = channel->getSocket();
	auto reactor = MSCast<KCPClientReactor>(channel->getReactor());
	asio::error_code error;
	auto length = socket->send_to(asio::buffer(buf, len), channel->getEndpoint(), 0, error);
	if (error)
	{
		reactor->onDisconnect(channel->shared_from_this());
		return -1;
	}
	return length;
}