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
#include "BackendServer.h"
#include "PlayerService.h"
#include "SpaceService.h"
#include <OpenMS/Mailbox/Private/Mail.h>
#include <OpenMS/Server/Private/Service.h>
#include <gflags/gflags.h>
DEFINE_uint32(space, 0, "空间ID");

MSString BackendServer::identity() const
{
	return "backend";
}

void BackendServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	auto argc = IStartup::Argc;
	auto argv = IStartup::Argv;
	google::ParseCommandLineFlags(&argc, &argv, true);
	auto spaceID = FLAGS_space;
	if (spaceID == 0) this->shutdown();

	auto spaceService = MSNew<SpaceService>(spaceID);
	spaceService->bind("enterSpace", [=](uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("用户 %u 加入空间", userID);
		auto playerService = MSNew<PlayerService>(userID);
		mailHub->create("player:" + std::to_string(userID), playerService);
		co_return true;
	});
	spaceService->bind("leaveSpace", [=](uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("用户 %u 离开空间", userID);
		mailHub->cancel("player:" + std::to_string(userID));
		co_return true;
	});
	spaceService->bind("syncStatus", [=](uint32_t userID)->MSAsync<void>
	{
		co_return;
	});
	mailHub->create("space:" + std::to_string(spaceID), spaceService);
}

void BackendServer::onExit()
{
	ClusterServer::onExit();
}