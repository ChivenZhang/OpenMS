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

MSString RedisService::identity() const
{
	return "redis";
}

void RedisService::onInit()
{
	ClusterService::onInit();

	RedisClient::startup();

#if 1 // TEST

	MSString result;
	if(RedisClient::execute("del mykey", result))
	{
		MS_INFO("del mykey:%s", result.c_str());
	}
	if(RedisClient::execute("exists mykey", result))
	{
		MS_INFO("exist mykey:%s", result.c_str());
	}
	if(RedisClient::execute("set mykey Hello", result))
	{
		MS_INFO("set mykey:%s", result.c_str());
	}
	if(RedisClient::execute("get mykey", result))
	{
		MS_INFO("get mykey:%s", result.c_str());
	}

#endif
}

void RedisService::onExit()
{
	ClusterService::onExit();

	RedisClient::shutdown();
}

void RedisService::configureEndpoint(RedisClient::config_t &config)
{
	config.IP = "127.0.0.1";
	config.PortNum = 6379;
	config.UserName = {};
	config.Password = {};
}
