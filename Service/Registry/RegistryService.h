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
#include "Endpoint/RegistryServer.h"

class RegistryService
	:
	public Service,
	public RESOURCE2(Property, IProperty, "application"),
	public RESOURCE(RegistryServer, "registryServer")
{
public:
	void startup() override;
	void shutdown() override;
};