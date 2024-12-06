/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "RegistryService.h"

int main(int argc, char* argv[])
{
	return IApplication::Run<RegistryService>(argc, argv);
}

void RegistryService::onInit()
{
	AUTOWIRE(RegistryServer)::bean()->startup();
}

void RegistryService::onExit()
{
	AUTOWIRE(RegistryServer)::bean()->shutdown();
}
