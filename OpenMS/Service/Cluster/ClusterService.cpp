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
#include "ClusterService.h"

MSString ClusterService::identity() const
{
	return "cluster";
}

void ClusterService::onInit()
{
	if (AUTOWIRE(IProperty)::bean()->property(identity() + ".master").empty()) return;

	m_ClusterClient = MSNew<RPCClient>(RPCClient::config_t{
		.IP = property(identity() + ".master.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".master.port", 0U),
		.Workers = 1,
	});
	m_ClusterClient->startup();

	auto mails = AUTOWIRE(IMailContext)::bean();

	// Route remote mail to local

	m_MailServer = MSNew<RPCServer>(RPCServer::config_t{
		.IP = property(identity() + ".server.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".server.port", 0U),
		.Backlog = property(identity() + ".server.backlog", 0U),
		.Workers = property(identity() + ".server.workers", 0U),
	});
	m_MailServer->startup();
	m_MailServer->bind("mailbox", [=](uint32_t sid, MSString from, MSString to, MSString data)
	{
		mails->sendToMailbox({from, to, data, sid});
	});

	// Send mail to remote mailbox

	mails->sendToMailbox([this](IMail&& mail)-> bool
	{
		// Select remote address to send

		MSString address;
		{
			MSMutexLock lock(m_MailRouteLock);
			auto result = m_MailRouteMap.find(mail.To);
			if (result == m_MailRouteMap.end()) return false;
			auto& routes = result->second;
			address = routes[rand() % routes.size()];
		}
		if (address.empty()) return false;

		// Select an RPC client to send

		MSRef<RPCClient> mailClient;
		{
			MSMutexLock lock(m_MailClientLock);
			auto result = m_MailClientMap.emplace(address, nullptr);
			if (result.second || result.first->second == nullptr)
			{
				auto index = address.find(':');
				if (index == MSString::npos) return false;
				auto ip = address.substr(0, index);
				auto port = (uint16_t)std::stoul(address.substr(index + 1));
				result.first->second = MSNew<RPCClient>(RPCClient::config_t{
					.IP = ip,
					.PortNum = port,
					.Workers = 1,
				});
				result.first->second->startup();
			}
			mailClient = result.first->second;
		}
		if (mailClient == nullptr) return false;
		if (mailClient->connect() == false)
		{
			mailClient->shutdown();
			mailClient->startup();
		}
		if (mailClient->connect() == false) return false;
		return mailClient->call<void>("mailbox", 0, mail.SID, mail.From, mail.To, mail.Data);
	});

	// Push and pull mail table

	m_Heartbeat = startTimer(0, OPENMS_HEARTBEAT * 1000, [this](uint32_t handle)
	{
		if (m_ClusterClient->connect() == false)
		{
			m_ClusterClient->shutdown();
			m_ClusterClient->startup();
		}
		if (m_MailServer->connect() == false)
		{
			m_MailServer->shutdown();
			m_MailServer->startup();
		}
		if (m_ClusterClient->connect() == true && m_MailServer->connect() == true)
		{
			MSStringList mailList;
			AUTOWIRE(IMailContext)::bean()->listMailbox(mailList);
			MSString address = m_MailServer->address().lock()->getString();
			if (m_ClusterClient->call<bool>("push", 1000, address, mailList))
			{
				MSMutexLock lock(m_MailRouteLock);
				m_MailRouteMap = m_ClusterClient->call<MSMap<MSString, MSStringList>>("pull", 1000);
			}
		}
	});
}

void ClusterService::onExit()
{
	if (AUTOWIRE(IProperty)::bean()->property(identity() + ".master").empty()) return;

	if (m_Heartbeat) stopTimer(m_Heartbeat);
	m_Heartbeat = 0;

	if (m_MailServer) m_MailServer->shutdown();
	m_MailServer = nullptr;

	for (auto& client : m_MailClientMap) client.second->shutdown();
	m_MailClientMap.clear();
	m_MailRouteMap.clear();

	if (m_ClusterClient) m_ClusterClient->shutdown();
	m_ClusterClient = nullptr;
}
