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
#include "Endpoint/IEndpoint.h"
struct redisContext;

/// @brief Redis Pool Endpoint
class RedisPool : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 6379;
		MSString UserName;
		MSString Password;
		uint8_t Instance = 1;	// Maximum instance
		uint8_t Reconnect = 1;	// Reconnect times
	};

public:
	explicit RedisPool(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;
	
	bool execute(MSString const& command, MSLambda<void(bool update, MSString const& data)> result);

protected:
	const config_t m_Config;
	MSRef<ISocketAddress> m_Address;
	MSMutex m_MutexLock;
	MSMutexUnlock m_MutexUnlock;
	MSAtomic<bool> m_Running;
	MSList<MSThread> m_ThreadList;

	struct execute_t
	{
		MSString Command;
		MSLambda<void(bool update, MSString const& data)> Callback;
	};
	MSQueue<execute_t> m_ExecuteQueue;
};