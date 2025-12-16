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
#include "Server/IProperty.h"
#include "Reactor/TCP/TCPClientReactor.h"

/// @brief TCP Client Endpoint
class TCPClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		TCPClientReactor::callback_tcp_t Callback;
	};

public:
	explicit TCPClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

protected:
	const config_t m_Config;
	MSRef<TCPClientReactor> m_Reactor;
};