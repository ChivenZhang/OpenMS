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

	auto mails = AUTOWIRE(IMailHub)::bean();

	// Handle remote mail to local

	m_ServiceServer = MSNew<RPCServer>(RPCServer::config_t{
		.IP = property(identity() + ".server.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".server.port", 0U),
		.Backlog = property(identity() + ".server.backlog", 0U),
		.Workers = property(identity() + ".server.workers", 0U),
	});
	m_ServiceServer->bind("mailbox", [=](uint32_t from, uint32_t to, uint32_t date, uint32_t type, MSList<char> const& body)
	{
		IMail mail = {};
		mail.From = from;
		mail.To = to;
		mail.Date = date;
		mail.Type = type;
		mail.Body = MSStringView(body.data(), body.size());
		mails->send(mail);
	});
	m_ServiceServer->startup();

	// Handle local mail to remote

	mails->send([this](IMail mail)->bool
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

		MSRef<RPCClient> client;
		{
			MSMutexLock lock(m_MailClientLock);
			auto result = m_MailClientMap.emplace(MSHash(address), nullptr);
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
			client = result.first->second;
		}
		if (client == nullptr) return false;
		if (client->connect() == false)
		{
			client->shutdown();
			client->startup();
		}
		if (client->connect() == false) return false;

		return client->call<void>("mailbox", 0, mail.From, mail.To, mail.Date, mail.Type, MSList<char>{mail.Body.begin(), mail.Body.end()});
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
		onPush();
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

void ClusterServer::onPush()
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
		MSList<IMailBox::name_t> mailList;
		AUTOWIRE(IMailHub)::bean()->list(mailList);
		MSString address = m_ServiceServer->address().lock()->getString();
		if (m_ClusterClient->call<bool>("push", 1000, address, mailList).first)
		{
			MSMutexLock lock(m_MailRouteLock);
			m_MailRouteMap = m_ClusterClient->call<MSMap<uint32_t, MSStringList>>("pull", 1000).first;
		}
	}
}
