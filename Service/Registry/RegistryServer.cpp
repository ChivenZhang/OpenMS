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

	using iptable_t = TMap<TString, TVector<TString>>;
	bind("registry/query", [=]()->iptable_t {

		return {};
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
