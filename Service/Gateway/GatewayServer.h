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
#include "OpenMS/Endpoint/TCP/TCPServer.h"

class GatewayServer : public TCPServer, public AUTOWIRE(IProperty)
{
public:
	GatewayServer();
	~GatewayServer();
	void configureEndpoint(config_t & config) override;
};