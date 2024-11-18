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
#include "OpenMS/Reactor/TCP/TCPServerReactor.h"

class TCPServer : public Endpoint
{
public:
	struct config_t
	{
		std::string Address;
		uint16_t PortNum;
		uint32_t Backlog;
		size_t WorkerNum;
		TCPServerReactor::callback_tcp_t Callback;
	};

public:
	void startup() override;
	void shutdown() override;
	virtual void configureEndpoint(config_t& config) = 0;

protected:
	TRef<TCPServerReactor> m_Reactor;
};