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
	TString ip;
	uint16_t port;
	uint32_t backlog;
	OPENMS_TYPE(RegistryServerConfig, ip, port, backlog)
};

RegistryServer::RegistryServer()
{
	m_IPTables = {
		{"service0", {"127.0.0.1:8080"}},
		{"service1", {"127.0.0.1:8081"}},
		{"service2", {"127.0.0.1:8082"}},
	};
	startup();
}

RegistryServer::~RegistryServer()
{
	shutdown();
}

void RegistryServer::configureEndpoint(config_t& config)
{
	auto property = AUTOWIRE(IProperty)::bean();
	auto configInfo = property->property<RegistryServerConfig>("registry.server");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;

	// See https://github.com/Netflix/eureka/wiki/Eureka-REST-operations fore more details

	bind("registry/query", [=]()->RegistryIPTable {
		TMutexLock lock(m_Lock);
		return m_IPTables;
		});

	bind("registry/register", [=](TString ip, uint16_t port)->TString {

		return "ok";
		});

	bind("registry/renew", [=](TString ip, uint16_t port)->TString {

		return "ok";
		});

	bind("registry/cancel", [=](TString ip, uint16_t port)->TString {

		return "ok";
		});
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
	auto property = AUTOWIRE(IProperty)::bean();
	auto configInfo = property->property<RegistryServerConfig>("registry.master");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
}
