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
#include "TCPServer.h"

void TCPServer::startup()
{
	config_t config;
	configureEndpoint(config);
	m_Reactor = TNew<TCPServerReactor>(
		IPv4Address::New(config.Address, config.PortNum), config.Backlog, config.WorkerNum,
		TCPServerReactor::callback_tcp_t{
		[=](TRef<IChannel> channel) {
			TPrint("New connection!");

		},
		[=](TRef<IChannel> channel) {
			TPrint("Disconnection !");

		},
		});
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("failed to start reactor");
}

void TCPServer::shutdown()
{
	m_Reactor->shutdown();
}