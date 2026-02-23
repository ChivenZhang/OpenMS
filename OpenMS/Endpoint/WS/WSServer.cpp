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
#include "WSServer.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

WSServer::WSServer(config_t const& config)
	:
	m_Config(config)
{
}

void WSServer::startup()
{
	if (m_Running == true) return;
	m_Running = true;

	MSPromise<void> promise;
	auto future = promise.get_future();

	m_EventThread = std::thread([this, &promise]() mutable
	{
		asio::io_context loop;
		websocketpp::server<websocketpp::config::asio> server;
		server.set_open_handler([this, &server](websocketpp::connection_hdl session)
		{
			websocketpp::lib::error_code error;
			auto connect = server.get_con_from_hdl(session, error);
			if (connect == nullptr || error)
			{
				MS_ERROR("failed to get_con_from_hdl: %s", error.message().c_str());
				return;
			}
			auto remote = connect->get_remote_endpoint();
			if (m_ConnectCallback) m_ConnectCallback(remote);
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
			auto remote = connect->get_remote_endpoint();
			if (m_DisconnectCallback) m_DisconnectCallback(remote);
		});
		server.set_message_handler([this](websocketpp::connection_hdl session, decltype(server)::message_ptr message)
		{
			if (message->get_opcode() == websocketpp::frame::opcode::BINARY)
			{
				if (m_BinaryCallback) m_BinaryCallback(message->get_payload());
				if (m_BinaryFrameCallback) m_BinaryFrameCallback(message->get_payload(), message->get_fin());
			}
			else if (message->get_opcode() == websocketpp::frame::opcode::TEXT)
			{
				if (m_MessageCallback) m_MessageCallback(message->get_payload());
				if (m_MessageFrameCallback) m_MessageFrameCallback(message->get_payload(), message->get_fin());
			}
		});

		websocketpp::lib::error_code error;
		server.init_asio(&loop, error);
		if (error)
		{
			MS_ERROR("failed to init_asio: %s", error.message().c_str());
			return;
		}
		server.listen(asio::ip::address::from_string(m_Config.IP), m_Config.PortNum, error);
		if (error)
		{
			MS_ERROR("failed to listen: %s", error.message().c_str());
			return;
		}
		server.start_accept(error);
		if (error)
		{
			MS_ERROR("failed to start_accept: %s", error.message().c_str());
			return;
		}

		MS_INFO("listening on %s:%u", m_Config.IP.c_str(), m_Config.PortNum);

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

		MS_INFO("closed server");
	});

	future.wait();
}

void WSServer::shutdown()
{
	if (m_Running == false) return;

	m_Running = false;
	if (m_EventThread.joinable()) m_EventThread.join();
}

bool WSServer::running() const
{
	return m_Running;
}

bool WSServer::connect() const
{
	return m_Connect;
}

MSHnd<IChannelAddress> WSServer::address() const
{
	return MSHnd<IChannelAddress>();
}

void WSServer::bind_connect(MSLambda<void(MSStringView address)> callback)
{
	m_ConnectCallback = callback;
}

void WSServer::bind_disconnect(MSLambda<void(MSStringView address)> callback)
{
	m_DisconnectCallback = callback;
}

void WSServer::bind_binary(MSLambda<void(MSStringView binary)> callback)
{
	m_BinaryCallback = callback;
}

void WSServer::bind_message(MSLambda<void(MSStringView message)> callback)
{
	m_MessageCallback = callback;
}

void WSServer::bind_binary(MSLambda<void(MSStringView frame, bool last)> callback)
{
	m_BinaryFrameCallback = callback;
}

void WSServer::bind_message(MSLambda<void(MSStringView frame, bool last)> callback)
{
	m_MessageFrameCallback = callback;
}
