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

class RegistryService : public Service
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
};