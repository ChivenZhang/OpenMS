#pragma once
/*=================================================
* Copyright © 2020-2026 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../IEndpoint.h"
#include "../../Reactor/WS/WSClientReactor.h"

class WSClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		MSString Path;
		uint32_t Workers = 0;
		WSClientReactor::callback_t Callback;
	};

public:
	explicit WSClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

protected:
	config_t m_Config;
	MSRef<WSClientReactor> m_Reactor;
};