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
#include <OpenMS/Endpoint/MySQL/MySQLClient.h>
#include <OpenMS/Endpoint/MySQL/MySQLPool.h>
#include "MySQLConfig.h"

class MySQLServer
	:
	public ClusterServer,
	public MySQLConfig
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	MSRef<MySQLPool> m_MysqlPool;
};

OPENMS_RUN(MySQLServer)