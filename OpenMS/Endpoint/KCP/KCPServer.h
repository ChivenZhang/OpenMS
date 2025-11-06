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
#include "OpenMS/Server/IProperty.h"
#include "Reactor/KCP/KCPServerReactor.h"

/// @brief KCP Server Endpoint
class KCPServer : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Backlog = 0;
		uint32_t Workers = 0;
		KCPServerReactor::callback_kcp_t Callback;
	};

public:
	explicit KCPServer(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

protected:
	const config_t m_Config;
	MSRef<KCPServerReactor> m_Reactor;
};