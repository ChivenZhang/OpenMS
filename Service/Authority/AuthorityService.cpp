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
#include "AuthorityService.h"
#include "OpenMS/Toolkit/Timer.h"
#include <OpenMS/Service/IStartup.h>

int openms_main(int argc, char** argv)
{
	AuthorityService service;
	service.startup();
	return 0;
}

bool running = false;

int AuthorityService::startup()
{
	running = true;
	signal(SIGINT, [](int) { running = false; });

	auto serviceName = property("authority.name");
	auto timer = AUTOWIRE(Timer)::bean();
	auto server = AUTOWIRE(AuthorityServer)::bean();
	auto client = AUTOWIRE(AuthorityClient)::bean();

	server->startup();
	client->startup();

	auto address = server->address().lock();
	if (address) client->call<TString>("registry/register", 1000, serviceName, address->getString());

	auto update_func = [=](uint32_t handle) {
		auto address = server->address().lock();
		if (address)  client->call<TString>("registry/renew", 100, serviceName, address->getString());
		client->async<TString>("registry/query", 100, {}, [](TString&& result) {
			TPrint("query result: %s", result.c_str());
			});
		};
	auto timerID = timer->start(0, 10, update_func);

	while (running)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	timer->stop(timerID);

	server->shutdown();
	client->shutdown();

	return 0;
}