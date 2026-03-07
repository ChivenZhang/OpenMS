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
#include "WSClientReactor.h"
#include "WSChannel.h"
#include "WSChannelEvent.h"
#include "WSChannelAddress.h"
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

WSClientReactor::WSClientReactor(MSRef<IWebSocketAddress> address, size_t workerNum, const callback_t& callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(address)
{
	if (m_Address == nullptr || m_Address->getAddress().empty()) m_Address = MSNew<WSIPv4Address>("0.0.0.0", 0);
}

void WSClientReactor::startup()
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
			websocketpp::client<websocketpp::config::asio_client> client;
			client.clear_access_channels(websocketpp::log::alevel::all);

			websocketpp::lib::error_code error;
			client.init_asio(&loop, error);
			if (error)
			{
				MS_ERROR("failed to init_asio: %s", error.message().c_str());
				break;
			}

			client.set_open_handler([this, &client](websocketpp::connection_hdl session)
			{
				MS_INFO("connected to %s", m_Address->getString().c_str());

				websocketpp::lib::error_code error;
				auto connect = client.get_con_from_hdl(session, error);
				if (connect == nullptr || error)
				{
					MS_ERROR("failed to get_con_from_hdl: %s", error.message().c_str());
					client.stop();
					return;
				}

				// Get the actual ip and port number

				MSRef<IWebSocketAddress> localAddress, remoteAddress;
				if (true)
				{
					{
						auto address = client.get_local_endpoint(error).address().to_string();
						auto portNum = client.get_local_endpoint(error).port();
						auto family = client.get_local_endpoint(error).protocol().family();
						if (family == AF_INET) localAddress = MSNew<WSIPv4Address>(address, portNum, connect->get_resource());
						else if (family == AF_INET6) localAddress = MSNew<WSIPv6Address>(address, portNum, connect->get_resource());
						else MS_ERROR("unknown address family: %d", family);
					}
					{
						auto address = connect->get_raw_socket().remote_endpoint().address().to_string();
						auto portNum = connect->get_raw_socket().remote_endpoint().port();
						auto family = connect->get_raw_socket().remote_endpoint().protocol().family();
						if (family == AF_INET) remoteAddress = MSNew<WSIPv4Address>(address, portNum);
						else if (family == AF_INET6) remoteAddress = MSNew<WSIPv6Address>(address, portNum);
						else MS_ERROR("unknown address family: %d", family);
					}
					if (localAddress == nullptr || remoteAddress == nullptr)
					{
						client.stop();
						return;
					}
					m_LocalAddress = localAddress;
				}

				MS_INFO("listening on %s:%d", m_LocalAddress->getAddress().c_str(), m_LocalAddress->getPort());

				auto channel = MSNew<WSChannel>(this, localAddress, remoteAddress, (uint32_t)(rand() % this->m_WorkerList.size()), session);
				this->onConnect(channel);
			});
			client.set_close_handler([this](websocketpp::connection_hdl session)
			{
				this->onDisconnect(m_Channel);
			});
			client.set_message_handler([this](websocketpp::connection_hdl session, decltype(client)::message_ptr message)
			{
				auto event = WSChannelEvent::New(message->get_payload(), (WSChannelEvent::opcode_t)message->get_opcode(), m_Channel);
				this->onInbound(event);
			});

			auto connect = client.get_connection("ws://" + m_Address->getString(), error);
			if (error)
			{
				MS_ERROR("failed to get_connection: %s", error.message().c_str());
				break;
			}
			client.connect(connect);

			m_FireSend = [&](MSRef<IChannelEvent> event)
			{
				loop.post([=, &client]()
				{
					auto _event = reinterpret_cast<WSChannelEvent*>(event.get());
					if (auto channel = MSCast<WSChannel>(_event->Channel.lock()))
					{
						client.send(channel->getHandle(), _event->Message, (websocketpp::frame::opcode::value)_event->OpCode);
					}
				});
			};

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

			// Close channel

			{
				if (m_Channel) onDisconnect(m_Channel);
				m_Channel = nullptr;
			}

			MS_INFO("closed client");
			return;

		} while (false);

		promise.set_value();
	});

	future.wait();
}

void WSClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
	m_Channel = nullptr;
}

MSHnd<IChannelAddress> WSClientReactor::address() const
{
	return m_Connect ? m_LocalAddress : MSHnd<IChannelAddress>();
}

void WSClientReactor::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	auto channel = m_Channel;
	if (channel && channel->running()) channel->write(event);
}

void WSClientReactor::onConnect(MSRef<Channel> channel)
{
	MS_DEBUG("accepted from %s", channel->getRemote().lock()->getString().c_str());

	m_Channel = channel;
	ChannelReactor::onConnect(channel);
}

void WSClientReactor::onDisconnect(MSRef<Channel> channel)
{
	MS_INFO("rejected from %s", channel->getRemote().lock()->getString().c_str());

	m_Channel = nullptr;
	ChannelReactor::onDisconnect(channel);
}

void WSClientReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	if (event == nullptr || event->Channel.expired()) return;
	if (m_Connect) m_FireSend(event);
}
