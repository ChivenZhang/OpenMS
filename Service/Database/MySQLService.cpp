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
#include "MySQLService.h"

// ========================================================================================

MSString MySQLService::identity() const
{
	return "mysql";
}

void MySQLService::onInit()
{
	ClusterService::onInit();
}

void MySQLService::onExit()
{
	ClusterService::onExit();
}