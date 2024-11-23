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

	bind("registry/register", [=](TString service, TString address)->TString {
		TMutexLock lock(m_Lock);
		m_IPTables[service].push_back(address);
		return "ok";
		});

	bind("registry/renew", [=](TString service, TString address)->TString {
		TPrint("renew %s:%s", service.c_str(), address.c_str());
		return "ok";
		});

	bind("registry/cancel", [=](TString service, TString address)->TString {

		return "ok";
		});
}