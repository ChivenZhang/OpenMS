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
#include "BusinessServer.h"
#include "LogicService.h"
#include <OpenMS/Mailbox/Private/Mail.h>
#include <OpenMS/Server/Private/Service.h>

#include "LoginService.h"

MSString BusinessServer::identity() const
{
	return "business";
}

void BusinessServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	// Timeout & Disconnect

	// startTimer(0, 5000, [=, this, self = logicService.get()](uint32_t handle)
	// {
	// 	MSMutexLock lock(m_UserLock);
	// 	auto now = ::clock() * 1.0f / CLOCKS_PER_SEC;
	// 	for (auto& userInfo : m_UserInfos)
	// 	{
	// 		if (userInfo.second.Online && userInfo.second.LastUpdate + 10.0f <= now)
	// 		{
	// 			self->call<bool>("logic", "logout", "", 0, MSTuple{ userInfo.first });
	// 		}
	// 	}
	// });

	auto loginService = MSNew<LoginService>();
	mailHub->create("login", loginService);

	auto logicService = MSNew<LogicService>();
	mailHub->create("logic", logicService);
}

void BusinessServer::onExit()
{
	ClusterServer::onExit();
}