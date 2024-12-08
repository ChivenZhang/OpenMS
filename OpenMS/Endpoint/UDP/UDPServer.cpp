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
#include "UDPServer.h"

void UDPServer::startup()
{
	config_t config;
	configureEndpoint(config);
	m_Reactor = MSNew<UDPServerReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Backlog,
		config.Broadcast,
		config.Multicast,
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void UDPServer::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool UDPServer::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool UDPServer::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> UDPServer::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}
