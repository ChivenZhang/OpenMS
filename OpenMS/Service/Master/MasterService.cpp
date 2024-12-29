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

	m_MailUpdateTime = ::clock();

	bind("push", [=](MSString address, MSList<MSString> mails)->bool
	{
		for (auto& mail : mails) m_MailRouteMap[mail].insert(address);
		for (auto& mail : mails) m_MailRouteNewMap[mail].insert(address);
		return true;
	});

	bind("pull", [=]()->MSStringMap<MSSet<MSString>>
	{
		auto now = ::clock();
		if (m_MailUpdateTime + 10000 <= now)
		{
			m_MailUpdateTime = now;
			m_MailRouteMap = m_MailRouteNewMap;
			m_MailRouteNewMap.clear();
		}
		return m_MailRouteMap;
	});
}

void MasterService::onExit()
{
	m_MailRouteNewMap.clear();
	RPCServer::shutdown();
}
