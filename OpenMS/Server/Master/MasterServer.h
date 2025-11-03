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
#include "OpenMS/Server/Private/Server.h"
#include "MasterConfig.h"
#include "Endpoint/RPC/RPCServer.h"

/// @brief Base Master Service
class MasterServer : public Server, public RESOURCE(MasterConfig)
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	MSRef<RPCServer> m_ClusterServer;
	MSMap<uint32_t, MSSet<MSString>> m_MailRouteMap;
	MSMap<uint32_t, MSSet<MSString>> m_MailRouteNewMap;
	std::chrono::time_point<std::chrono::system_clock> m_MailUpdateTime;
};
