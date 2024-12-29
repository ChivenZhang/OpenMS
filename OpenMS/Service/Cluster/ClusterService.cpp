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

void ClusterService::onInit()
{
	RPCClient::startup();

	// Push mail address to master

	startTimer(0, 10000, [=](uint32_t handle)
	{
		if (connect() == false)
		{
			RPCClient::shutdown();
			RPCClient::startup();
		}
		if (connect() == true)
		{
			MSStringList mailList;
			AUTOWIRE(IMailContext)::bean()->listMailbox(mailList);
			if (call<bool>("push", 1000, address().lock()->getString(), mailList))
			{
				m_MailRouteMap = call<MSMultiMap<MSString, MSString>>("pull", 1000);
				for (auto& mail : m_MailRouteMap)
				{
					MS_INFO("%s@%s", mail.first.c_str(), mail.second.c_str());
				}
				MS_INFO("sync mail table success");
			}
		}
	});
}

void ClusterService::onExit()
{
	RPCClient::shutdown();
}

void ClusterService::configureEndpoint(config_t& config) const
{
	auto ip = property("cluster.server.ip");
	auto port = property("cluster.server.port", 0U);
	config.IP = ip;
	config.PortNum = port;
	config.Workers = 1;
}
