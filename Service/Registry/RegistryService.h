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
#include "OpenMS/Private/Service.h"
#include <csignal>

class RegistryService : public Service
{
public:
	void startup() override;
	void shutdown() override;
};