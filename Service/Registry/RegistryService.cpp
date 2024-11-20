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
#include <OpenMS/Service/IBootstrap.h>

TRef<IService> openms_bootstrap()
{
	return TNew<RegistryService>();
}

void RegistryService::startup()
{
	Service::startup();

	auto client = AUTOWIREN(RegistryClient, "registry-client")::bean();
	client->invoke<void>("print", "你好，服务器！");
}

void RegistryService::shutdown()
{
	Service::shutdown();
}