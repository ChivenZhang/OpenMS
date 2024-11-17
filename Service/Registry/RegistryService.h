#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "OpenMS/Service/Private/Service.h"
#include "OpenMS/Service/Private/Property.h"

class RegistryService : public Service, public RESOURCE2(Property, IProperty, "application")
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
};