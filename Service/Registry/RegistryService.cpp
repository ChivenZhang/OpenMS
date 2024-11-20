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

	server->bind("echo", TLambda([=](TString text) {
		TPrint("%s", text.c_str());
		}));
	server->bind("add", TLambda([=](int a, int b) {
		return a + b;
		}));

	client->call<TString>("echo", "你好，服务器！");

	auto result = client->call<int>("add", 234, 432);
	TPrint("计算结果：%d", result);
}

void RegistryService::shutdown()
{
	Service::shutdown();
}