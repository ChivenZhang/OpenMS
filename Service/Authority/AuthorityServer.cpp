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
#include "AuthorityServer.h"
#include <OpenMS/Reactor/Private/ChannelHandler.h>

struct AuthorityServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	OPENMS_TYPE(AuthorityServerConfig, ip, port, backlog)
};

AuthorityServer::AuthorityServer()
{
	startup();
}

AuthorityServer::~AuthorityServer()
{
	shutdown();
}

void AuthorityServer::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE(IProperty)::bean();
	auto configInfo = properties->property<AuthorityServerConfig>("authority.server");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;
}

AuthorityClient::AuthorityClient()
{
	startup();
}

AuthorityClient::~AuthorityClient()
{
	shutdown();
}

void AuthorityClient::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE(IProperty)::bean();
	auto configInfo = properties->property<AuthorityServerConfig>("authority.master");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
}