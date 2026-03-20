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
#include "OpenMS/Endpoint/RPC/RPCServer.h"

/// @brief Base Master Service
class MasterServer : public Server, public RESOURCE(MasterConfig)
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	struct info_t
	{
		MSString MailIPAddr;
		MSList<uint32_t> MailTables;
	};
	MSMutex m_LockClient;
	MSRef<RPCServer> m_ClusterServer;
	MSMap<MSRef<IChannel>, info_t> m_ClientInfoMap;
};
