#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "../Private/Endpoint.h"
#include "OpenMS/Service/IProperty.h"
#include "OpenMS/Reactor/UDP/UDPClientReactor.h"

class UDPClient : public Endpoint
{
public:
	struct config_t
	{
		std::string IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		bool Broadcast = false;
		bool Multicast = false;
		UDPClientReactor::callback_udp_t Callback;
	};

public:
	void startup() override;
	void shutdown() override;
	virtual void configureEndpoint(config_t& config) = 0;

protected:
	TRef<UDPClientReactor> m_Reactor;
};