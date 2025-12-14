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
#include <OpenMS/Server/Cluster/ClusterServer.h>
#include "Endpoint/TCP/TCPClient.h"

class BusinessServer : public ClusterServer
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	MSMutex m_UserLock;
	MSMutex m_BattleLock;
	struct userinfo_t
	{
		bool Online;
		uint32_t SpaceID;
		float LastUpdate;
	};
	MSQueue<uint32_t> m_MatchQueue;
	MSMap<uint32_t, userinfo_t> m_UserInfos;
};

OPENMS_RUN(BusinessServer)