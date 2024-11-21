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

	server->bind("echo", [=](TString text) {
		TPrint("%s", text.c_str());
		});
	server->bind("add", [=](int a, int b) {
		return a + b;
		});

	client->call<void>("echo", "计算1+2+...+100");
	auto sum = 0;
	for (auto i = 1; i <= 100; ++i)
	{
		sum = client->call<int>("add", sum, i);
	}
	client->call<void>("echo", std::to_string(sum));
}

void RegistryService::shutdown()
{
	Service::shutdown();
}