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
#include "RegistryService.h"
#include <OpenMS/Service/IStartup.h>

TRef<IService> openms_startup()
{
	return TNew<RegistryService>();
}

void RegistryService::startup()
{
	AUTOWIRE(RegistryServer)::bean()->startup();
}

void RegistryService::shutdown()
{
	AUTOWIRE(RegistryServer)::bean()->shutdown();
}