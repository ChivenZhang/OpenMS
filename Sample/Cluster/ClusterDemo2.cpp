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
		this->bind("verify", [](MSStringView input)->MSAsync<MSString>
		{
			// MS_INFO("success %s", input.data());
			co_return "success";
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

	auto hub = AUTOWIRE(IMailHub)::bean();
	hub->create("author", MSNew<AuthorService>());
}

void ClusterDemo2::onExit()
{
	ClusterServer::onExit();
}