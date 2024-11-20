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
#include "RemoteClient.h"

void RemoteClient::startup()
{
	config_t config;
	configureEndpoint(config);
	config.Workers = 0;
	config.Callback =
	{
		// TODO: implement callback functions
	};
	m_Reactor = TNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("failed to start reactor");
}

void RemoteClient::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}
