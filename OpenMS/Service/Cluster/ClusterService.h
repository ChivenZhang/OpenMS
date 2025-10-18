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
#include "Service/Private/Service.h"
#include "ClusterConfig.h"
#include "Endpoint/RPC/RPCClient.h"
#include "Endpoint/RPC/RPCServer.h"

/// @brief Base Cluster Service
class ClusterService
	:
	public Service,
	public RESOURCE(ClusterConfig),
	public AUTOWIRE(IMailContext)
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
	MSRef<RPCServer> m_MailServer;
	MSRef<RPCClient> m_ClusterClient;
	MSStringMap<MSStringList> m_MailRouteMap;
	MSStringMap<MSRef<RPCClient>> m_MailClientMap;
};
