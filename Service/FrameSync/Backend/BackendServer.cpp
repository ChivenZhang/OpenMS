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
#include "SpaceService.h"
#include <OpenMS/Mailbox/Private/Mail.h>
#include <OpenMS/Server/Private/Service.h>
#include <gflags/gflags.h>
DEFINE_uint32(game, 0, "游戏ID");
DEFINE_uint32(space, 0, "空间ID");
DEFINE_string(caller, "null", "调用者");

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
	auto gameID = FLAGS_game;
	auto spaceID = FLAGS_space;
	auto caller = FLAGS_caller;
	if (spaceID == 0)
	{
		MS_ERROR("invalid space id");
		this->shutdown();
		return;
	}

	auto spaceService = this->onCreatingSpace(spaceID, gameID);
	mailHub->create("space:" + std::to_string(spaceID), spaceService);

	spaceService->call<void>(spaceService->name(), "onCreatingSpace", "", 0, MSTuple{caller,});

	this->startTimer(5000, 0, [this, self = spaceService.get()]()
	{
		self->call<void>(self->name(), "endPlay", "", 5000, MSTuple{});
	});
}

void BackendServer::onExit()
{
	ClusterServer::onExit();
}

MSRef<Service> BackendServer::onCreatingSpace(uint32_t spaceID, uint32_t gameID)
{
	return MSNew<SpaceService>(spaceID, gameID);
}
