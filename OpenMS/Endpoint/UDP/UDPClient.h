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
#include "Reactor/UDP/UDPClientReactor.h"

/// @brief UDP Client Endpoint
class UDPClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		bool Broadcast = false;
		bool Multicast = false;
		UDPClientReactor::callback_udp_t Callback;
	};

public:
	explicit UDPClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

protected:
	const config_t m_Config;
	MSRef<UDPClientReactor> m_Reactor;
};