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
#include "RedisServer.h"

MSString RedisServer::identity() const
{
	return "redis";
}

void RedisServer::onInit()
{
	ClusterServer::onInit();

	m_RedisPool = MSNew<RedisPool>(RedisPool::config_t{
		.IP = property(identity() + ".redis.ip", MSString("127.0.0.1")),
		.PortNum  = (uint16_t)property(identity() + ".redis.port", 6379U),
		.UserName = property(identity() + ".redis.username", MSString()),
		.Password = property(identity() + ".redis.password", MSString()),
		.Instance = (uint8_t)property(identity() + ".redis.instance", 1U),
		.Reconnect = (uint8_t)property(identity() + ".redis.reconnect", 1U),
	});
	m_RedisPool->startup();

#if 0 // TEST
	for (size_t i = 0; i < 10; ++i)
	{
		m_RedisPool->execute("exists mykey" + std::to_string(i), [](bool update, MSString const& result)
		{
			MS_INFO("del mykey:%s", result.c_str());
		});
		m_RedisPool->execute("del mykey" + std::to_string(i), [](bool update, MSString const& result)
		{
			MS_INFO("del mykey:%s", result.c_str());
		});
		m_RedisPool->execute("hmset mykey" + std::to_string(i) + " a1 b1 a2 b2", [](bool update, MSString const& result)
		{
			MS_INFO("set mykey:%s", result.c_str());
		});
		m_RedisPool->execute("hgetall mykey" + std::to_string(i), [](bool update, MSString const& result)
		{
			MS_INFO("get mykey:%s", result.c_str());
		});
	}
#endif
}

void RedisServer::onExit()
{
	ClusterServer::onExit();

	if (m_RedisPool) m_RedisPool->shutdown();
	m_RedisPool = nullptr;
}