#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "OpenMS/Service/Private/Service.h"
#include "OpenMS/Service/Private/PropertySource.h"

class RegistryService : public Service, public RESOURCE2(PropertySource, IPropertySource, "application")
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
};