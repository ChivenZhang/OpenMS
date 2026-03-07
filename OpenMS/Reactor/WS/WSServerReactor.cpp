/*=================================================
* Copyright © 2020-2026 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "WSServerReactor.h"
#include "WSChannel.h"
#include "WSChannelEvent.h"
#include "WSChannelAddress.h"
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

WSServerReactor::WSServerReactor(MSRef<IWebSocketAddress> address, uint32_t backlog, size_t workerNum, const callback_t& callback)
	:
	ChannelReactor(workerNum, callback),
	m_Backlog(backlog ? backlog : 128),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<WSIPv4Address>("0.0.0.0", 0);
}

void WSServerReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = MSThread([=, this, &promise]()
	{
		do
		{
			asio::io_context loop;
			websocketpp::server<websocketpp::config::asio> server;
			server.clear_access_channels(websocketpp::log::alevel::all);

			websocketpp::lib::error_code error;
			server.init_asio(&loop, error);
			if (error)
			{
				MS_ERROR("failed to init_asio: %s", error.message().c_str());
				break;
			}

			server.set_open_handler([this, &server](websocketpp::connection_hdl session)
			{
				websocketpp::lib::error_code error;
				auto connect = server.get_con_from_hdl(session, error);
				if (connect == nullptr || error)
				{
					MS_ERROR("failed to get_con_from_hdl: %s", error.message().c_str());
					return;
				}

				MSRef<IWebSocketAddress> localAddress, remoteAddress;
				{
					auto address = server.get_local_endpoint(error).address().to_string();
					auto portNum = server.get_local_endpoint(error).port();
					auto family = server.get_local_endpoint(error).protocol().family();
					if (family == AF_INET) localAddress = MSNew<WSIPv4Address>(address, portNum);
					else if (family == AF_INET6) localAddress = MSNew<WSIPv6Address>(address, portNum);
					else MS_ERROR("unknown address family: %d", family);
				}
				{
					auto address = connect->get_raw_socket().remote_endpoint().address().to_string();
					auto portNum = connect->get_raw_socket().remote_endpoint().port();
					auto family = connect->get_raw_socket().remote_endpoint().protocol().family();
					if (family == AF_INET) remoteAddress = MSNew<WSIPv4Address>(address, portNum, connect->get_resource());
					else if (family == AF_INET6) remoteAddress = MSNew<WSIPv6Address>(address, portNum, connect->get_resource());
					else MS_ERROR("unknown address family: %d", family);
				}
				if (localAddress == nullptr || remoteAddress == nullptr)
				{
					return;
				}

				auto channel = MSNew<WSChannel>(this, localAddress, remoteAddress, (uint32_t)(rand() % this->m_WorkerList.size()), session);
				this->onConnect(channel);
			});
			server.set_close_handler([this, &server](websocketpp::connection_hdl session)
			{
				websocketpp::lib::error_code error;
				auto connect = server.get_con_from_hdl(session, error);
				if (connect == nullptr || error)
				{
					MS_ERROR("failed to get_con_from_hdl: %s", error.message().c_str());
					return;
				}

				auto result = m_ChannelMap.find(session.lock().get());
				if (result == m_ChannelMap.end())
				{
					MS_ERROR("failed to find channel for session");
					return;
				}

				auto channel = result->second;
				this->onDisconnect(channel);
			});
			server.set_message_handler([this, &server](websocketpp::connection_hdl session, decltype(server)::message_ptr message)
			{
				auto result = m_ChannelMap.find(session.lock().get());
				if (result == m_ChannelMap.end()) return;
				auto event = WSChannelEvent::New(message->get_payload(), (WSChannelEvent::opcode_t)message->get_opcode(), result->second);
				this->onInbound(event);
			});

			server.set_listen_backlog(m_Backlog);
			server.listen(asio::ip::address::from_string(m_Address->getAddress()), m_Address->getPort(), error);
			if (error)
			{
				MS_ERROR("failed to listen: %s", error.message().c_str());
				break;
			}

			// Get the actual ip and port number

			if (true)
			{
				MSRef<IWebSocketAddress> localAddress;
				auto address = server.get_local_endpoint(error).address().to_string();
				auto portNum = server.get_local_endpoint(error).port();
				auto family = server.get_local_endpoint(error).protocol().family();
				if (family == AF_INET) localAddress = MSNew<WSIPv4Address>(address, portNum);
				else if (family == AF_INET6) localAddress = MSNew<WSIPv6Address>(address, portNum);
				else MS_ERROR("unknown address family: %d", family);
				if (localAddress == nullptr) break;
				m_LocalAddress = localAddress;
			}

			MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

			m_FireSend = [&](MSRef<IChannelEvent> event)
			{
				loop.post([=, &server]()
				{
					auto _event = reinterpret_cast<WSChannelEvent*>(event.get());
					if (auto channel = MSCast<WSChannel>(_event->Channel.lock()))
					{
						server.send(channel->getHandle(), _event->Message, (websocketpp::frame::opcode::value)_event->OpCode);
					}
				});
			};

			server.start_accept(error);
			if (error)
			{
				MS_ERROR("failed to start_accept: %s", error.message().c_str());
				break;
			}

			// Run the event loop

			asio::steady_timer timer(loop);
			MSLambda<void()> timer_func;
			timer_func = [&]()
			{
				timer.expires_after(std::chrono::milliseconds(500));
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
			m_FireSend = nullptr;

			{
				auto channels = m_ChannelMap;
				for (auto& channel : channels)
				{
					if (channel.second) this->onDisconnect(channel.second);
				}
				m_ChannelMap.clear();
			}

			MS_INFO("closed server");
			return;

		} while (false);

		promise.set_value();
	});

	future.wait();
}

void WSServerReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_ChannelMap.clear();
}

MSHnd<IChannelAddress> WSServerReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void WSServerReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false || event == nullptr) return;
	auto channel = event->Channel.lock();
	if (channel && channel->running()) channel->write(event);
}

void WSServerReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	auto _channel = MSCast<WSChannel>(channel);
	m_ChannelMap[_channel->getHandle().lock().get()] = channel;
	ChannelReactor::onConnect(channel);
}

void WSServerReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_DEBUG("rejected from %s", channel->getRemote().lock()->getString().c_str());

	auto _channel = MSCast<WSChannel>(channel);
	m_ChannelMap.erase(_channel->getHandle().lock().get());
	ChannelReactor::onDisconnect(channel);
}

void WSServerReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	if (event == nullptr || event->Channel.expired()) return;
	if (m_Connect) m_FireSend(event);
}
