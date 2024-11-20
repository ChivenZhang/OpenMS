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
#include "RegistryConfig.h"

class RegistryService :
	public Service,
	public RESOURCE(RegistryConfig),
	public AUTOWIREN(IEndpoint, "registry-server"),
	public AUTOWIREN(RegistryClient, "registry-client")
{
public:
	void startup() override;
	void shutdown() override;
};