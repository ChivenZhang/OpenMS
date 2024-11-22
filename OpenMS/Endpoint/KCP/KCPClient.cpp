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
#include "KCPClient.h"

void KCPClient::startup()
{
	config_t config;
	configureEndpoint(config);
	m_Reactor = TNew<KCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("failed to start reactor");
}

void KCPClient::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool KCPClient::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool KCPClient::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

THnd<IChannelAddress> KCPClient::address() const
{
	return m_Reactor ? m_Reactor->address() : THnd<IChannelAddress>();
}
