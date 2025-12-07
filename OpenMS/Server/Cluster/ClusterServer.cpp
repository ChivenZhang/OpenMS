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

#include <utility>

#include "Mailbox/Private/Mail.h"

MSString ClusterServer::identity() const
{
	return "cluster";
}

void ClusterServer::onInit()
{
	if (AUTOWIRE(IProperty)::bean()->property(identity() + ".master").empty()) return;
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	// Handle local mail to remote

	mailHub->send([this](IMail mail)->bool
	{
		// Select remote address to send

		MSString address;
		{
			MSMutexLock lock(m_MailRouteLock);
			auto toName = (mail.Type & OPENMS_MAIL_TYPE_FORWARD) ? mail.Copy : mail.To;
			auto result = m_MailRouteMap.find(toName);
			if (result == m_MailRouteMap.end()) return false;
			auto& routes = result->second;
			address = routes.front();
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
		if (client && client->connect() == false)
		{
			client->shutdown();
			client->startup();
		}
		if (client == nullptr || client->connect() == false) return false;

		MSString request(sizeof(MailView) + mail.Body.size(), 0);
		auto& mailView = *(MailView*)request.data();
		mailView.From = mail.From;
		mailView.To = mail.To;
		mailView.Copy = mail.Copy;
		mailView.Date = mail.Date;
		mailView.Type = mail.Type;
		if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
		MSString response;
		return client->call("mailbox", 0, request, response);
	});

	// Handle remote mail to local

	m_ServiceServer = MSNew<RPCServer>(RPCServer::config_t{
		.IP = property(identity() + ".server.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".server.port", 0U),
		.Backlog = property(identity() + ".server.backlog", 0U),
		.Workers = property(identity() + ".server.workers", 0U),
	});
	m_ServiceServer->bind("mailbox", [=](MSHnd<IChannel> client, MSStringView const& request, MSString& response)->bool
	{
		if (sizeof(MailView) <= request.size())
		{
			auto& mailView = *(MailView*)request.data();
			IMail newMail = {};
			newMail.From = mailView.From;
			newMail.To = mailView.To;
			newMail.Copy = mailView.Copy;
			newMail.Date = mailView.Date;
			newMail.Type = mailView.Type;
			newMail.Body = MSStringView(mailView.Body, request.size() - sizeof(MailView));
			mailHub->send(newMail);
			return true;
		}
		return false;
	});
	m_ServiceServer->startup();

	// Connect to master server

	m_ClusterClient = MSNew<RPCClient>(RPCClient::config_t{
		.IP = property(identity() + ".master.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".master.port", 0U),
	});
	m_ClusterClient->bind("pull", [this](MSMap<uint32_t, MSStringList> route)
	{
		MSMutexLock lock(m_MailRouteLock);
		m_MailRouteMap = std::move(route);
		MS_INFO("validate %s, count %u", m_ServiceServer->address().lock()->getString().c_str(), (uint32_t)m_MailRouteMap.size());
	});
	m_ClusterClient->startup();

	// Push and pull route table

	this->onPush();
	m_Heartbeat = startTimer(OPENMS_HEARTBEAT * 1000, OPENMS_HEARTBEAT * 1000, [this](uint32_t handle)
	{
		this->onPush();
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
		MSMutexLock lock(m_MailRouteLock);
		auto result = m_ClusterClient->call<MSMap<uint32_t, MSStringList>>("push", 1000, address, mailList);
		if (result.second) m_MailRouteMap = result.first;
		MS_INFO("validate %s, count %u", address.c_str(), (uint32_t)m_MailRouteMap.size());
	}
}
