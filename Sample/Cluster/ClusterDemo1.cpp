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
				this->async("author", "verify", "", 1000, MSTuple{user, pass}, [=](MSString response)
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

	// RPC : login.login() => author.verify()
	auto response = loginService->call<MSString>("login", "login", "", 1000, MSTuple{"admin", "123456"});
	MS_INFO("Login result: %s", response.first.c_str());
}

void ClusterDemo1::onExit()
{
	ClusterServer::onExit();
}
