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
#include "PlayerService.h"
#include "ServerService.h"
#include "Mailbox/Private/Mail.h"
#include "Server/Private/Service.h"

MSString BusinessServer::identity() const
{
	return "business";
}

void BusinessServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	auto logicService = MSNew<Service>();
	logicService->bind("login", [=, this](MSString user, MSString pass)->MSAsync<uint32_t>
	{
		MS_INFO("服务端：LOGIN!!!");
		static uint32_t s_UserID = 0;
		auto userID = ++s_UserID;

		auto serverService = MSNew<ServerService>();
		serverService->bind("readyBattle", [=, this]()->MSAsync<bool>
		{
			MS_INFO("服务端：READY BATTLE!!!"); // TODO：Match battle

			auto playerService = MSNew<PlayerService>();
			playerService->bind("startBattle", [=]()->MSAsync<bool>
			{
				MS_INFO("用户 %u 开始游戏", userID);
				playerService->call<void>("client:" + std::to_string(userID), "onStartBattle", "proxy:" + std::to_string(userID), 0, MSTuple{});
				co_return true;
			});
			playerService->bind("stopBattle", [=]()->MSAsync<bool>
			{
				MS_INFO("用户 %u 结束游戏", userID);
				playerService->call<void>("client:" + std::to_string(userID), "onStopBattle", "proxy:" + std::to_string(userID), 0, MSTuple{});
				co_return true;
			});
			playerService->bind("attack", [=]()->MSAsync<bool>
			{
				MS_INFO("用户 %u 发动攻击", userID);
				playerService->call<void>("client:" + std::to_string(userID), "onAttack", "proxy:" + std::to_string(userID), 0, MSTuple{});
				co_return true;
			});
			if (mailHub->create("player:" + std::to_string(userID), playerService)) this->onPush();

#if 1 // TEST start battle
			co_return co_await [=](MSAwait<bool> promise)
			{
				MS_INFO("服务端：START BATTLE!!!");
				playerService->async("player:" + std::to_string(userID), "startBattle", "", 100, MSTuple{}, [=](bool result)
				{
					MS_INFO("服务端：STARTED BATTLE %d", result);
					promise(result);
				});
			};
#else
			co_return true;
#endif
		});
		if (mailHub->create("server:" + std::to_string(userID), serverService)) this->onPush();
		co_return userID;
	});
	logicService->bind("logout", [=](uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("服务端：LOGOUT!!!");	// TODO: 心跳超时，自动登出
		if (mailHub->cancel("player:" + std::to_string(userID))) this->onPush();
		if (mailHub->cancel("server:" + std::to_string(userID))) this->onPush();
		co_return true;
	});
	logicService->bind("signup", [](MSString user, MSString pass)->MSAsync<bool>
	{
		MS_INFO("服务端：SIGNUP!!!");
		co_return false;
	});
	if (mailHub->create("logic", logicService)) this->onPush();
}

void BusinessServer::onExit()
{
	ClusterServer::onExit();
}