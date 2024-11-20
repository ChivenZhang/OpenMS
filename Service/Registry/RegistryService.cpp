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

	auto server = AUTOWIREN(RegistryServer, "registry-server")::bean();
	auto client = AUTOWIREN(RegistryClient, "registry-client")::bean();

	server->bind("print", TLambda([=](TString text) {
		TPrint("%s", text.c_str());
		return "执行成功！";
		}));

	auto result = client->invoke<TString>("print", "你好，服务器！");
}

void RegistryService::shutdown()
{
	Service::shutdown();
}