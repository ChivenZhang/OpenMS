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

class LoginService : public Service
{
public:
	LoginService()
	{
		this->bind("login", [this](MSStringView input)->MSAsync<MSString>
		{
			auto output = co_await [=](MSAwait<MSString> promise)->MSString
			{
				this->async("author", "verify", 100, input, [=](MSStringView output)
				{
					promise(MSString(output));
				});
				return {};
			};
			// MS_INFO("RPC %s", output.c_str());
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
		while (m_Running)
		{
			MSString response;
			loginService->call("login", "login", 1000, "{}", response);
		}
	});
}

void ClusterDemo1::onExit()
{
	m_Running = false;
	if (m_Thread.joinable()) m_Thread.join();

	ClusterServer::onExit();
}
