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
#include <OpenMS/Service/Cluster/ClusterService.h>
#include <OpenMS/Endpoint/Redis/RedisClient.h>
#include "RedisConfig.h"

class RedisService 
	:
	public ClusterService,
	public RedisClient,
	public RedisConfig
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	void configureEndpoint(RedisClient::config_t& config) override;
};

OPENMS_RUN(RedisService)