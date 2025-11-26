/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "ClusterDemo1.h"
#include "Server/Private/Service.h"
#include "Utility/QuickQPS.h"

class LoginService : public Service
{
public:
	LoginService()
	{
		this->bind("login", [this](MSString user, MSString pass)->MSAsync<MSString>
		{
			auto output = co_await [=](MSAwait<MSString> promise)
			{
				this->async("author", "verify", 1000, MSTuple{user, pass}, [=](MSString response)
				{
					promise(MSString(response));
				});
			};
			co_return output;
		});
	}
};

// ========================================================================================

MSString ClusterDemo1::identity() const
{
	// Use config in APPLICATION.json
	return "cluster1";
}

void ClusterDemo1::onInit()
{
	ClusterServer::onInit();

	auto hub = AUTOWIRE(IMailHub)::bean();
	auto loginService = MSNew<LoginService>();
	hub->create("login", loginService);

	m_Running = true;
	m_Thread = MSThread([=, this]()
	{
		QuickQPS qps;
		auto T = ::clock();

		while (m_Running)
		{
			auto response = loginService->call<MSString>("author", "verify", 1000, MSTuple{"admin", "123456"});
			MS_DEBUG("Get %s", response.first.c_str());

			qps.hit();
			auto t = ::clock();
			if (T + 5000 <= t)
			{
				MS_INFO("QPS %f", qps.get());
				T = t;
			}
		}
	});
}

void ClusterDemo1::onExit()
{
	m_Running = false;
	if (m_Thread.joinable()) m_Thread.join();

	ClusterServer::onExit();
}
