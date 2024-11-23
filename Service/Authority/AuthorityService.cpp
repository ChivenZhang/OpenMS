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

TRef<IService> openms_startup()
{
	return TNew<AuthorityService>();
}

TMutex mutex;
TMutexUnlock unlock;
bool running = false;

void AuthorityService::startup()
{
	running = true;
	signal(SIGINT, [](int) {
		running = false;
		unlock.notify_all();
		});
	AUTOWIRE(AuthorityServer)::bean()->startup();
	AUTOWIRE(AuthorityClient)::bean()->startup();


	auto server = AUTOWIRE(AuthorityServer)::bean();
	auto client = AUTOWIRE(AuthorityClient)::bean();
	auto address = server->address().lock();
	if (address)
	{
		client->call<TString>("registry/register", "authority", address->getString());
	}

	Timer timer;
	auto update_func = [=]() {
		auto address = server->address().lock();
		if (address == nullptr) return;
		auto serviceName = property("authority.name");
		client->call<TString>("registry/renew", serviceName, address->getString());
		auto result = client->call<TString>("registry/query");
		TPrint("query result: %s", result.c_str());
		};
	auto timerID = timer.start(0, 5000, update_func);

	while (running)
	{
		timer.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	timer.stop(timerID);

	AUTOWIRE(AuthorityServer)::bean()->shutdown();
	AUTOWIRE(AuthorityClient)::bean()->shutdown();
}