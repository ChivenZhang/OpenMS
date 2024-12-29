/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 15:57:15.
*
* =================================================*/
#include "ClusterService.h"

MSString ClusterService::identity() const
{
	return "cluster";
}

void ClusterService::configureEndpoint(config_t& config) const
{
	auto ip = property(identity() + ".server.ip");
	auto port = property(identity() + ".server.port", 0U);
	config.IP = ip;
	config.PortNum = port;
	config.Workers = 1;
}

void ClusterService::onInit()
{
	RPCClient::startup();

	auto context = AUTOWIRE(IMailContext)::bean();

	// Route remote mail to local

	auto ip = property(identity() + ".server.ip");
	m_MailServer = MSNew<ClusterServer>(ip, 0, 0, 0);
	m_MailServer->startup();
	m_MailServer->bind("mailbox", [=](uint32_t sid, MSString from, MSString to, MSString data)
	{
		context->sendToMailbox({from, to, data, sid });
	});

	// Send mail to remote mailbox

	context->sendToMailbox([=](IMail&& mail)-> bool
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

		MSRef<ClusterClient> client;
		{
			MSMutexLock lock(m_MailClientLock);
			auto result = m_MailClientMap.emplace(address, nullptr);
			if (result.second || result.first->second == nullptr)
			{
				auto index = address.find(':');
				if (index == std::string::npos) return false;
				auto ip = address.substr(0, index);
				auto port = static_cast<uint16_t>(std::stoul(address.substr(index + 1)));
				result.first->second = MSNew<ClusterClient>(ip, port);
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

		return client->call<void>("mailbox", 0, mail.SID, mail.From, mail.To, mail.Data);
	});

	// Push mail address to master

	startTimer(0, 10000, [=](uint32_t handle)
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
				MS_INFO("pulled mail table");
			}
		}
	});
}

void ClusterService::onExit()
{
	if (m_MailServer) m_MailServer->shutdown();
	m_MailServer = nullptr;

	RPCClient::shutdown();
}
