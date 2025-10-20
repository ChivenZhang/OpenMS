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
#include "ClusterServer.h"

MSString ClusterServer::identity() const
{
	return "cluster";
}

void ClusterServer::onInit()
{
	if (AUTOWIRE(IProperty)::bean()->property(identity() + ".master").empty()) return;

	auto mails = AUTOWIRE(IMailContext)::bean();

	// Handle remote mail to local

	m_ServiceServer = MSNew<RPCServer>(RPCServer::config_t{
		.IP = property(identity() + ".server.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".server.port", 0U),
		.Backlog = property(identity() + ".server.backlog", 0U),
		.Workers = property(identity() + ".server.workers", 0U),
	});
	m_ServiceServer->bind("mailbox", [=](MSString from, MSString to, MSString data, uint32_t fromSID, uint32_t toSID)
	{
		mails->sendToMailbox({from, to, data, fromSID, toSID});
	});
	m_ServiceServer->startup();

	// Handle local mail to remote

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
				result.first->second = MSNew<RPCClient>(RPCClient::config_t{
					.IP = address.substr(0, index),
					.PortNum = (uint16_t)std::stoul(address.substr(index + 1)),
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
		return mailClient->call<void>("mailbox", 0, mail.From, address + ":" + mail.To, mail.Data, mail.FromSID, mail.ToSID);
	});

	// Connect to master server

	m_ClusterClient = MSNew<RPCClient>(RPCClient::config_t{
		.IP = property(identity() + ".master.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".master.port", 0U),
	});
	m_ClusterClient->startup();

	// Push and pull route table

	m_Heartbeat = startTimer(0, OPENMS_HEARTBEAT * 1000, [this](uint32_t handle)
	{
		if (m_ClusterClient->connect() == false)
		{
			m_ClusterClient->shutdown();
			m_ClusterClient->startup();
		}
		if (m_ServiceServer->connect() == false)
		{
			m_ServiceServer->shutdown();
			m_ServiceServer->startup();
		}
		if (m_ClusterClient->connect() == true && m_ServiceServer->connect() == true)
		{
			MSStringList mailList;
			AUTOWIRE(IMailContext)::bean()->listMailbox(mailList);
			MSString address = m_ServiceServer->address().lock()->getString();
			if (m_ClusterClient->call<bool>("push", 1000, address, mailList))
			{
				MSMutexLock lock(m_MailRouteLock);
				m_MailRouteMap = m_ClusterClient->call<MSMap<MSString, MSStringList>>("pull", 1000);
			}
		}
	});
}

void ClusterServer::onExit()
{
	if (AUTOWIRE(IProperty)::bean()->property(identity() + ".master").empty()) return;

	if (m_Heartbeat) stopTimer(m_Heartbeat);
	m_Heartbeat = 0;

	if (m_ServiceServer) m_ServiceServer->shutdown();
	m_ServiceServer = nullptr;

	if (m_ClusterClient) m_ClusterClient->shutdown();
	m_ClusterClient = nullptr;

	for (auto& client : m_MailClientMap) client.second->shutdown();
	m_MailRouteMap.clear();
	m_MailClientMap.clear();
}
