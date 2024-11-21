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

	server->bind("sleep", [=](uint32_t ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		});
	server->bind("sleep2", [=](uint32_t ms)->TString {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		return "success";
		});
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

	client->call<void>("echo", "进入睡眠");
	client->async<void>("sleep", std::tuple{ 1000 }, []() {
		TPrint("1 second passed");
		});
	client->call<void>("echo", "结束睡眠");

	client->call<void>("echo", "进入睡眠");
	client->async<TString>("sleep2", std::tuple{ 1000 }, [](TString result) {
		TPrint("1 second passed");
		TPrint("返回结果: %s", result.c_str());
		});
	client->call<void>("echo", "结束睡眠");
}

void RegistryService::shutdown()
{
	Service::shutdown();
}