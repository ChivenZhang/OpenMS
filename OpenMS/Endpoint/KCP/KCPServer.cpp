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
#include "KCPServer.h"

void KCPServer::startup()
{
	config_t config;
	configureEndpoint(config);
	m_Reactor = TNew<KCPServerReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Backlog,
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("failed to start reactor");
}

void KCPServer::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool KCPServer::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool KCPServer::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

THnd<IChannelAddress> KCPServer::address() const
{
	return m_Reactor ? m_Reactor->address() : THnd<IChannelAddress>();
}
