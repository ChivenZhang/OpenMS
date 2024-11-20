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
#include <OpenMS/Reactor/Private/ChannelHandler.h>

struct RegistryServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	OPENMS_TYPE(RegistryServerConfig, ip, port, backlog)
};

RegistryServer::RegistryServer()
{
	bind(TString("print"), TLambda([=](TString request) {
		TPrint("%s", request.c_str());
		}));

	startup();
}

RegistryServer::~RegistryServer()
{
	shutdown();
}

void RegistryServer::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE(IProperty)::bean();
	auto configInfo = properties->property<RegistryServerConfig>("registry.server");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;
}

RegistryClient::RegistryClient()
{
	startup();
}

RegistryClient::~RegistryClient()
{
	shutdown();
}

void RegistryClient::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE(IProperty)::bean();
	auto configInfo = properties->property<RegistryServerConfig>("registry.master");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
}
