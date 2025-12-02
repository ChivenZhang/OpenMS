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
		.Workers = 1U,
	});
	m_ClusterServer->startup();

	// Maintain mail route table

	m_MailUpdateTime = std::chrono::system_clock::now();
	m_ClusterServer->bind("push", [this](MSHnd<IChannel> client, MSString const& address, MSList<IMailBox::name_t> const& mails)
	{
		auto channel = client.lock();
		m_MailClientSet.insert(channel);
		m_MailClientNewSet.insert(channel);
		for (auto& mail : mails) m_MailRouteMap[mail].insert(address);
		for (auto& mail : mails) m_MailRouteNewMap[mail].insert(address);
		auto now = std::chrono::system_clock::now();
		if (OPENMS_HEARTBEAT <= std::chrono::duration_cast<std::chrono::seconds>(now - m_MailUpdateTime).count())
		{
			m_MailUpdateTime = now;
			m_MailRouteMap = std::move(m_MailRouteNewMap);
			m_MailClientSet = std::move(m_MailClientNewSet);
		}
		// 广播同步路由表
		for (auto& E : m_MailClientSet)
		{
			if (E == channel) continue;
			m_ClusterServer->call<void>(E, "pull", 0, m_MailRouteMap);
		}
		MS_INFO("validate %s", address.c_str());
		return m_MailRouteMap;
	});
}

void MasterServer::onExit()
{
	if (m_ClusterServer) m_ClusterServer->shutdown();
	m_ClusterServer = nullptr;

	m_MailClientSet.clear();
	m_MailClientNewSet.clear();
	m_MailRouteMap.clear();
	m_MailRouteNewMap.clear();
}