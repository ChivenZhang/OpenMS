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
#include "Server/Private/Service.h"

class AuthorService : public Service
{
public:
	AuthorService()
	{
		this->bind("verify", [](MSString user, MSString pass)->MSAsync<MSString>
		{
			MS_DEBUG("success %s %s", user.c_str(), pass.c_str());
			co_return "SUCCESS";
		});
	}
};

// ========================================================================================

MSString ClusterDemo2::identity() const
{
	// Use config in APPLICATION.json
	return "cluster2";
}

void ClusterDemo2::onInit()
{
	ClusterServer::onInit();

	auto mailHub = AUTOWIRE(IMailHub)::bean();
	mailHub->create("author", MSNew<AuthorService>());
}

void ClusterDemo2::onExit()
{
	ClusterServer::onExit();
}