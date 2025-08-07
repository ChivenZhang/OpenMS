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

	RedisPool::startup();

#if 1 // TEST
	for (size_t i = 0; i < 10; ++i)
	{
		RedisPool::execute("exists mykey" + std::to_string(i), [](bool update, MSString const& result)
		{
			MS_INFO("del mykey:%s", result.c_str());
		});
		RedisPool::execute("del mykey" + std::to_string(i), [](bool update, MSString const& result)
		{
			MS_INFO("del mykey:%s", result.c_str());
		});
		RedisPool::execute("hmset mykey" + std::to_string(i) + " a1 b1 a2 b2", [](bool update, MSString const& result)
		{
			MS_INFO("set mykey:%s", result.c_str());
		});
		RedisPool::execute("hgetall mykey" + std::to_string(i), [](bool update, MSString const& result)
		{
			MS_INFO("get mykey:%s", result.c_str());
		});
	}
#endif
}

void RedisService::onExit()
{
	ClusterService::onExit();

	RedisPool::shutdown();
}

void RedisService::configureEndpoint(RedisPool::config_t& config)
{
	config.IP = "127.0.0.1";
	config.PortNum = 6379;
	config.UserName = {};
	config.Password = {};
	config.Instance = 2;
}
