#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
* 
* 
* ====================History=======================
* Created by chivenzhang@gmail.com.
* 
* =================================================*/
#include "OpenMS/Endpoint/IEndpoint.h"
struct redisContext;

class RedisClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 6379;
		MSString UserName;
		MSString Password;
	};

public:
	explicit RedisClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;
	
	bool execute(MSString const& cmd, MSString& result);

protected:
	config_t m_Config;
	MSRef<ISocketAddress> m_Address;
	MSRaw<redisContext> m_Context = nullptr;
};