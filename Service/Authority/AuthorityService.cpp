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

void AuthorityService::startup()
{
	auto serviceName = property("authority.name");

	m_Running = true;
	m_Thread = TThread([=]() {
		auto server = AUTOWIRE(AuthorityServer)::bean();
		auto client = AUTOWIRE(AuthorityClient)::bean();


		auto update_func = [=]() {
			auto address = server->address().lock();
			if (address == nullptr) return;
			client->call<TString>("registry/renew", serviceName, address->getString());
			auto result = client->call<TString>("registry/query");
			TPrint("query result: %s", result.c_str());
			};

		Timer timer;
		auto timerID = timer.start(0, 5000, update_func);

		while (m_Running)
		{
			timer.update();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		timer.stop(timerID);

		});

	auto server = AUTOWIRE(AuthorityServer)::bean();
	auto client = AUTOWIRE(AuthorityClient)::bean();
	auto address = server->address().lock();
	if (address)
	{
		auto result = client->call<TString>("registry/register", "authority", address->getString());
	}
}

void AuthorityService::shutdown()
{
	m_Running = false;
	m_Unlock.notify_one();
	if (m_Thread.joinable()) m_Thread.join();
}