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
#include "RegistryService.h"

void RegistryService::startup(int argc, char** argv)
{
	Service::startup(argc, argv);
}

void RegistryService::shutdown()
{
	Service::shutdown();
}

#include <OpenMS/Service/Bootstrap.h>

TRef<IService> openms_service()
{
	return TNew<RegistryService>();
}