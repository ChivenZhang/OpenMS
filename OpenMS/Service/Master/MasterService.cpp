/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "MasterService.h"

MSString MasterService::identity() const
{
	return "master";
}

void MasterService::onInit()
{
	m_RPCServer = MSNew<RPCServer>(RPCServer::config_t{
		.IP = property(identity() + ".server.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".server.port", 0U),
		.Backlog = property(identity() + ".server.backlog", 0U),
		.Workers = 1,
	});
	m_RPCServer->startup();

	// Register or update mail address

	m_MailUpdateTime = std::chrono::system_clock::now();

	m_RPCServer->bind("push", [this](MSString address, MSList<MSString> mails)->bool
	{
		for (auto& mail : mails) m_MailRouteMap[mail].insert(address);
		for (auto& mail : mails) m_MailRouteNewMap[mail].insert(address);
		return true;
	});

	m_RPCServer->bind("pull", [this]()->MSStringMap<MSSet<MSString>>
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

	if (m_RPCServer) m_RPCServer->shutdown();
	m_RPCServer = nullptr;
}