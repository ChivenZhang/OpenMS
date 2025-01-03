#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 15:57:15.
*
* =================================================*/
#include "../Private/Service.h"
#include "ClusterConfig.h"
#include "ClusterClient.h"
#include "ClusterServer.h"

/// @brief 
class ClusterService
	:
	public Service,
	public RPCClient,
	public RESOURCE(ClusterConfig),
	public AUTOWIRE(IMailContext)
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	void configureEndpoint(config_t& config) const final;

protected:
	MSMutex m_MailRouteLock;
	MSMutex m_MailClientLock;
	MSRef<ClusterServer> m_MailServer;
	MSStringMap<MSStringList> m_MailRouteMap;
	MSStringMap<MSRef<ClusterClient>> m_MailClientMap;
};
