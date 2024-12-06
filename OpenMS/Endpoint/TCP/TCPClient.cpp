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
#include "TCPClient.h"

void TCPClient::startup()
{
	config_t config;
	configureEndpoint(config);
	m_Reactor = MSNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MSFatal("failed to start reactor");
}

void TCPClient::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool TCPClient::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool TCPClient::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> TCPClient::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}
