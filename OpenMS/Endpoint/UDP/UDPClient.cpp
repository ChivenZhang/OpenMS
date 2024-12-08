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
#include "UDPClient.h"

void UDPClient::startup()
{
	config_t config;
	configureEndpoint(config);
	m_Reactor = MSNew<UDPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Broadcast,
		config.Multicast,
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void UDPClient::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool UDPClient::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool UDPClient::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> UDPClient::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}
