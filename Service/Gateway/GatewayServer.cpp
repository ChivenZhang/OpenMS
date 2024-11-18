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
#include "GatewayServer.h"
#include "GatewayHandler.h"

struct GatewayServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	uint32_t workers;
	OPENMS_TYPE(GatewayServerConfig, ip, port, backlog, workers)
};

GatewayServer::GatewayServer()
{
	startup();
}

GatewayServer::~GatewayServer()
{
	shutdown();
}

void GatewayServer::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE_DATA(IProperty);
	auto configInfo = properties->property<GatewayServerConfig>("gateway.server");
	config.Address = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;
	config.WorkerNum = configInfo.workers;
	config.Callback = {
		[=](TRef<IChannel> channel) {
			TPrint("new connection");
			channel->getPipeline()->addFirst("", TNew<GatewayInboundHandler>());
			channel->getPipeline()->addFirst("", TNew<GatewayOutboundHandler>());
		},
		[=](TRef<IChannel> channel) {
			TPrint("disconnection ");
		},
	};
}
