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
#include <OpenMS/Service/IBootstrap.h>

TRef<IService> openms_bootstrap()
{
	return TNew<AuthorityService>();
}

void AuthorityService::startup()
{
	Service::startup();

	m_Running = true;
	m_Thread = TThread([=]() {
		auto server = RESOURCE(AuthorityServer)::bean();
		auto client = RESOURCE(AuthorityClient)::bean();
		while (m_Running)
		{
			TUniqueLock lock(m_Lock);
			auto address = server->address().lock();
			if (address)
			{
				auto result = client->call<TString>("registry/renew", "authority", address->getString());
				result = client->call<TString>("registry/query");
				TPrint("query result: %s", result.c_str());
			}
			m_Unlock.wait_for(lock, std::chrono::seconds(2), [=]() { return m_Running == false; });
		}
		});

	auto server = RESOURCE(AuthorityServer)::bean();
	auto client = RESOURCE(AuthorityClient)::bean();
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
	Service::shutdown();
}