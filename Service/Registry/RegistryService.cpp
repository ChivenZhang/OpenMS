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

#ifdef OPENMS_TEST

	auto server = AUTOWIREN(RegistryServer, "registry-server")::bean();
	auto client = AUTOWIREN(RegistryClient, "registry-client")::bean();

	server->bind("echo", [=](TString text) {
		TPrint("%s", text.c_str());
		});
	server->bind("add", [=](int a, int b) {
		return a + b;
		});
	server->bind("sleep", [=](uint32_t ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		});

	client->call<void>("echo", "计算1+2+...+100");
	auto sum = 0;
	for (auto i = 1; i <= 100; ++i)
	{
		sum = client->call<int>("add", sum, i);
	}
	client->call<void>("echo", std::to_string(sum));

	client->call<void>("echo", "进入睡眠");
	client->async<void>("sleep", std::tuple{ 1000 }, []() {
		TPrint("1 second passed");
		});
	client->call<void>("echo", "结束睡眠");

#endif

	auto iptable = client->call<RegistryIPTable>("registry/query");
	for (auto& [ip, services] : iptable)
	{
		TPrint("%s: ", ip.c_str());
		for (auto& service : services) TPrint("%s ", service.c_str());
	}
}

void RegistryService::shutdown()
{
	Service::shutdown();
}