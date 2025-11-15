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
#include "MasterServer.h"
#include "Mailbox/IMailBox.h"

MSString MasterServer::identity() const
{
	return "master";
}

void MasterServer::onInit()
{
	m_ClusterServer = MSNew<RPCServer>(RPCServer::config_t{
		.IP = property(identity() + ".server.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".server.port", 0U),
		.Backlog = property(identity() + ".server.backlog", 0U),
	});
	m_ClusterServer->startup();

	// Maintain mail route table

	m_MailUpdateTime = std::chrono::system_clock::now();
	m_ClusterServer->bind("push", [this](MSString const& address, MSList<IMailBox::name_t> const& mails)->bool
	{
		for (auto& mail : mails) m_MailRouteMap[mail].insert(address);
		for (auto& mail : mails) m_MailRouteNewMap[mail].insert(address);
		MS_INFO("validate %s", address.c_str());
		return true;
	});

	m_ClusterServer->bind("pull", [this]()->MSMap<uint32_t, MSSet<MSString>>
	{
		auto now = std::chrono::system_clock::now();
		if (OPENMS_HEARTBEAT <= std::chrono::duration_cast<std::chrono::seconds>(now - m_MailUpdateTime).count())
		{
			m_MailUpdateTime = now;
			m_MailRouteMap = std::move(m_MailRouteNewMap);
			m_MailRouteNewMap.clear();
		}
		return m_MailRouteMap;
	});
}

void MasterServer::onExit()
{
	m_MailRouteMap.clear();
	m_MailRouteNewMap.clear();

	if (m_ClusterServer) m_ClusterServer->shutdown();
	m_ClusterServer = nullptr;
}