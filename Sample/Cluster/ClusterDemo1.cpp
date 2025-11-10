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
		this->bind("login1", [this](MSStringView input)->MSAsync<MSString>
		{
			MSString response;
			this->call("author", "verify", 100, input, response);
			co_return response;
		});

		this->bind("login2", [this](MSStringView input)->MSAsync<MSString>
		{
			auto response = co_await MSAwait<MSString>([input, this](MSAwait<MSString>::handle_t handle)
			{
				this->async("author", "verify", 100, input, [&](MSStringView output) mutable
				{
					handle.setValue("Test");
				});
			});
			MS_INFO("login %s", response.c_str());
			co_return response;
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
			loginService->call("login", "login2", 1000, R"({"user":"admin", "pass":"******"})", response);
			break;
		}
	});
}

void ClusterDemo1::onExit()
{
	m_Running = false;
	if (m_Thread.joinable()) m_Thread.join();

	ClusterServer::onExit();
}
