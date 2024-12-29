/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 15:56:12.
*
* =================================================*/
#include "MasterService.h"

MSString MasterService::identity() const
{
	return "master";
}

void MasterService::configureEndpoint(config_t& config) const
{
	auto ip = property(identity() + ".server.ip");
	auto port = property(identity() + ".server.port", 0U);
	config.IP = ip;
	config.PortNum = port;
	config.Workers = 1;
}

void MasterService::onInit()
{
	RPCServer::startup();

	// Register or update mail address

	bind("push", [=](MSString address, MSList<MSString> mails)->bool
	{
		for (auto& mail : mails)
		{
			m_MailRouteMap.insert({ mail, address });
		}
		return true;
	});

	bind("pull", [=]()->MSMultiMap<MSString, MSString>
	{
		return m_MailRouteMap;
	});
}

void MasterService::onExit()
{
	RPCServer::shutdown();
}
