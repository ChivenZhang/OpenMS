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
		.Callback
		{
			.OnOpen = [this](MSRef<IChannel> channel)
			{
				MSMutexLock lock(m_LockClient);
				m_ClientInfoMap[channel] = {};
			},
			.OnClose = [this](MSRef<IChannel> channel)
			{
				MSMutexLock lock(m_LockClient);
				m_ClientInfoMap.erase(channel);

				// 广播路由表
				MSMap<uint32_t, MSStringList> result;
				for (auto& infos : m_ClientInfoMap)
				{
					for (auto& mail : infos.second.MailTables)
					{
						result[mail].push_back(infos.second.MailIPAddr);
					}
				}
				for (auto& e : m_ClientInfoMap)
				{
					if (e.first == channel) continue;
					m_ClusterServer->call<void>(e.first, "pull", 0, result);
				}
			},
		},
	});
	m_ClusterServer->startup();

	// Maintain mail route table

	m_ClusterServer->bind("push", [this](MSHnd<IChannel> client, MSString const& address, MSList<uint32_t> const& mails)
	{
			MS_INFO("validate 0");
		MSMutexLock lock(m_LockClient);
		auto channel = client.lock();
		m_ClientInfoMap[channel].MailIPAddr = address;
		m_ClientInfoMap[channel].MailTables = mails;

		// 广播路由表
		MSMap<uint32_t, MSStringList> result;
		for (auto& infos : m_ClientInfoMap)
		{
			for (auto& mail : infos.second.MailTables)
			{
				result[mail].push_back(infos.second.MailIPAddr);
			}
		}
		for (auto& info : m_ClientInfoMap)
		{
			if (info.first == channel) continue;
			m_ClusterServer->call<void>(info.first, "pull", 0, result);
		}
		MS_INFO("validate %s, count %u", address.c_str(), (uint32_t)result.size());
		return result;
	});
}

void MasterServer::onExit()
{
	if (m_ClusterServer) m_ClusterServer->shutdown();
	m_ClusterServer = nullptr;

	m_ClientInfoMap.clear();
}