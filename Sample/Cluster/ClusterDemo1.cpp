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

MSString ClusterDemo1::identity() const
{
	return "cluster1";	// Config in APPLICATION.json
}

void ClusterDemo1::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	mailHub->create("authorService", MSNew<AuthorService>());
}

void ClusterDemo1::onExit()
{
	ClusterServer::onExit();
}

AuthorService::AuthorService()
{
	// Define remote call...

	this->bind("verify", [](MSString user, MSString pass)->MSAsync<MSString>
	{
		MS_INFO("验证登录（VERIFY） %s %s", user.c_str(), pass.c_str());
		co_return "登录成功（SUCCESS）";
	});
}
