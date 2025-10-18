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
#include "../Private/Endpoint.h"
#include "Service/IProperty.h"
#include "Reactor/KCP/KCPClientReactor.h"

class KCPClient : public Endpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		KCPClientReactor::callback_kcp_t Callback;
	};

public:
	explicit KCPClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

protected:
	config_t m_Config;
	MSRef<KCPClientReactor> m_Reactor;
};