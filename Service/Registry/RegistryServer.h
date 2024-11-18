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
#include "RegistryHandler.h"

class RegistryServer :
	public TCPServer,
	public AUTOWIRE(IProperty),
	public RESOURCE(RegistryInboundHandler),
	public RESOURCE(RegistryOutboundHandler)
{
public:
	RegistryServer();
	~RegistryServer();
	void configureEndpoint(config_t & config) override;
};