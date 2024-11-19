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
#include "OpenMS/Reactor/KCP/KCPServerReactor.h"

class KCPServer : public Endpoint
{
public:
	struct config_t
	{
		TString IP;
		uint16_t PortNum = 0;
		uint32_t Backlog = 0;
		uint32_t Workers = 0;
		KCPServerReactor::callback_kcp_t Callback;
	};

public:
	void startup() override;
	void shutdown() override;
	virtual void configureEndpoint(config_t& config) = 0;

protected:
	TRef<KCPServerReactor> m_Reactor;
};