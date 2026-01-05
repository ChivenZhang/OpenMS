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
#include "ClusterDemo2.h"

MSString ClusterDemo2::identity() const
{
	return "cluster2";	// Config in APPLICATION.json
}

void ClusterDemo2::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	auto loginService = MSNew<LoginService>();
	mailHub->create("loginService", loginService);

	// Invoke remote call...

	auto result = loginService->call<MSString>("loginService", "login", "", 1000, MSTuple{"admin", "123456"});
	MS_INFO("登录结果（Result）: %s", result.first.c_str());
}

void ClusterDemo2::onExit()
{
	ClusterServer::onExit();
}

LoginService::LoginService()
{
	// Define remote call...

	this->bind("login", [this](MSString user, MSString pass)->MSAsync<MSString>
	{
		auto result = co_await [=, this](MSAwait<MSString> promise)
		{
			this->async("authorService", "verify", "", 1000, MSTuple{user, pass}, [=](MSString response)
			{
				promise(MSString(response));
			});
		};
		co_return result;
	});
}