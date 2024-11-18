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
#include "RegistryServer.h"

struct RegistryServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	uint32_t workers;
	OPENMS_TYPE(RegistryServerConfig, ip, port, backlog, workers)
};

RegistryServer::RegistryServer()
{
	startup();
}

RegistryServer::~RegistryServer()
{
	shutdown();
}

void RegistryServer::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE_DATA(IProperty);
	auto configInfo = properties->property<RegistryServerConfig>("registry.server");
	config.Address = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;
	config.WorkerNum = configInfo.workers;
}
