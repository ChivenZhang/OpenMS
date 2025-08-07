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
#include <OpenMS/Endpoint/MySQL/MySQLClient.h>
#include <OpenMS/Endpoint/MySQL/MySQLPool.h>
#include "MySQLConfig.h"

class MySQLService
	:
	public ClusterService,
	public MySQLPool,
	public MySQLConfig
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	void configureEndpoint(MySQLPool::config_t& config) override;
};

OPENMS_RUN(MySQLService)