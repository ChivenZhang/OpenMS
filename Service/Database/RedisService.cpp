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
#include "RedisService.h"

// ========================================================================================

MSString RedisService::identity() const
{
	return "redis";
}

void RedisService::onInit()
{
	ClusterService::onInit();
}

void RedisService::onExit()
{
	ClusterService::onExit();
}