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
#include "WSClient.h"
#include "../../Reactor/WS/WSChannelAddress.h"

WSClient::WSClient(config_t const& config)
	:
	m_Config(config)
{
}

void WSClient::startup()
{
	auto config = m_Config;
	m_Reactor = MSNew<WSClientReactor>(
		WSIPv4Address::New(config.IP, config.PortNum, config.Path),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void WSClient::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool WSClient::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool WSClient::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> WSClient::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}
