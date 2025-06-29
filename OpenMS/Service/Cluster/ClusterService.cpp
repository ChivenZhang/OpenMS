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

	RPCClient::startup();

	auto mails = AUTOWIRE(IMailContext)::bean();

	// Route remote mail to local

	m_MailServer = MSNew<ClusterServer>(
		property(identity() + ".server.ip", MSString("127.0.0.1")),
		property(identity() + ".server.port", 0U),
		property(identity() + ".server.backlog", 0U),
		property(identity() + ".server.workers", 0U)
		);
	m_MailServer->startup();
	m_MailServer->bind("mailbox", [=](uint32_t sid, MSString from, MSString to, MSString data)
	{
		mails->sendToMailbox({from, to, data, sid });
	});

	// Send mail to remote mailbox

	mails->sendToMailbox([=](IMail&& mail)-> bool
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

		MSRef<ClusterClient> mailClient;
		{
			MSMutexLock lock(m_MailClientLock);
			auto result = m_MailClientMap.emplace(address, nullptr);
			if (result.second || result.first->second == nullptr)
			{
				auto index = address.find(':');
				if (index == MSString::npos) return false;
				auto ip = address.substr(0, index);
				auto port = std::stoul(address.substr(index + 1));
				result.first->second = MSNew<ClusterClient>(ip, port, 1);
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

	m_Heartbeat = startTimer(0, OPENMS_HEARTBEAT * 1000, [=](uint32_t handle)
	{
		if (connect() == false)
		{
			RPCClient::shutdown();
			RPCClient::startup();
		}
		if (m_MailServer->connect() == false)
		{
			m_MailServer->shutdown();
			m_MailServer->startup();
		}
		if (connect() == true && m_MailServer->connect() == true)
		{
			MSStringList mailList;
			AUTOWIRE(IMailContext)::bean()->listMailbox(mailList);
			MSString address = m_MailServer->address().lock()->getString();
			if (call<bool>("push", 1000, address, mailList))
			{
				MSMutexLock lock(m_MailRouteLock);
				m_MailRouteMap = call<MSMap<MSString, MSStringList>>("pull", 1000);
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

	RPCClient::shutdown();
}

void ClusterService::configureEndpoint(config_t& config)
{
	auto ip = property(identity() + ".master.ip", MSString("127.0.0.1"));
	auto port = property(identity() + ".master.port", 0U);
	config.IP = ip;
	config.PortNum = port;
	config.Workers = 1;
}