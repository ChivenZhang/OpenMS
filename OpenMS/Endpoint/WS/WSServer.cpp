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
#include "WSServer.h"
#include "OpenMS/Reactor/WS/WSChannelAddress.h"

WSServer::WSServer(config_t const& config)
	:
	m_Config(config)
{
}

void WSServer::startup()
{
	auto config = m_Config;
	m_Reactor = MSNew<WSServerReactor>(
		WSIPv4Address::New(config.IP, config.PortNum, "/"),
		config.Backlog,
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void WSServer::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool WSServer::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool WSServer::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> WSServer::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}
