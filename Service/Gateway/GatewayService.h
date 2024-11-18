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
#include "OpenMS/Service/Private/Service.h"
#include "OpenMS/Service/Private/Property.h"
#include "GatewayServer.h"

class GatewayService
	:
	public Service,
	public RESOURCE2(Property, IProperty),
	public RESOURCE(GatewayServer)
{
public:
	void startup() override;
	void shutdown() override;
};