/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
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

void MasterService::onInit()
{
	RPCServer::startup();

	// Register or update mail address

	m_MailUpdateTime = std::chrono::system_clock::now();

	bind("push", [=](MSString address, MSList<MSString> mails)->bool
	{
		for (auto& mail : mails) m_MailRouteMap[mail].insert(address);
		for (auto& mail : mails) m_MailRouteNewMap[mail].insert(address);
		return true;
	});

	bind("pull", [=]()->MSStringMap<MSSet<MSString>>
	{
		auto now = std::chrono::system_clock::now();
		if (OPENMS_HEARTBEAT <= std::chrono::duration_cast<std::chrono::seconds>(now - m_MailUpdateTime).count())
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
	m_MailRouteMap.clear();
	m_MailRouteNewMap.clear();

	RPCServer::shutdown();
}

void MasterService::configureEndpoint(config_t& config) const
{
	auto ip = property(identity() + ".server.ip", MSString("127.0.0.1"));
	auto port = property(identity() + ".server.port", 0U);
	config.IP = ip;
	config.PortNum = port;
	config.Workers = 1;
}