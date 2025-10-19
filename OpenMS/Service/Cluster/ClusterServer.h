#pragma once
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
#include "Service/Private/Server.h"
#include "ClusterConfig.h"
#include "Endpoint/RPC/RPCClient.h"
#include "Endpoint/RPC/RPCServer.h"

/// @brief Base Cluster Service
class ClusterServer : public Server, public RESOURCE(ClusterConfig), public AUTOWIRE(IMailContext)
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	uint32_t m_Heartbeat = 0;
	MSMutex m_MailRouteLock;
	MSMutex m_MailClientLock;
	MSRef<RPCServer> m_ServiceServer;
	MSRef<RPCClient> m_ClusterClient;
	MSStringMap<MSStringList> m_MailRouteMap;	// name : [ip+port]
	MSStringMap<MSRef<RPCClient>> m_MailClientMap;
};
