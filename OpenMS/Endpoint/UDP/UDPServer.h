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
#include "OpenMS/Service/IProperty.h"
#include "OpenMS/Reactor/UDP/UDPServerReactor.h"

class UDPServer : public Endpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Backlog = 0;
		uint32_t Workers = 0;
		bool Broadcast = false;
		bool Multicast = false;
		UDPServerReactor::callback_udp_t Callback;
	};

public:
	explicit UDPServer(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

protected:
	config_t m_Config;
	MSRef<UDPServerReactor> m_Reactor;
};