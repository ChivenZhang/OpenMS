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
}