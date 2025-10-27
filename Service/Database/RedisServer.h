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
#include <OpenMS/Service/Cluster/ClusterServer.h>
#include <OpenMS/Endpoint/Redis/RedisClient.h>
#include <OpenMS/Endpoint/Redis/RedisPool.h>
#include "RedisConfig.h"

class RedisServer : public ClusterServer, public RedisConfig
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	MSRef<RedisPool> m_RedisPool;
};

OPENMS_RUN(RedisServer)