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
#include "DaemonServer.h"
#include <OpenMS/Mailbox/Private/Mail.h>
#include <OpenMS/Server/Private/Service.h>

MSString DaemonServer::identity() const
{
	return "daemon";
}

void DaemonServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	auto daemonService = MSNew<Service>();
	daemonService->bind("createSpace", [=](uint32_t spaceID)->MSAsync<bool>
	{
		MS_INFO("守护进程：CREATE SPACE!!!");

#ifdef OPENMS_PLATFORM_WINDOWS
		auto result = system(("start BackendServer.exe --space=" + std::to_string(spaceID)).c_str());
#elifdef OPENMS_PLATFORM_APPLE
		auto result = system(("open BackendServer --args space=" + std::to_string(spaceID)).c_str());
#elifdef OPENMS_PLATFORM_LINUX
		auto result = system(("BackendServer --space=" + std::to_string(spaceID) + " &").c_str());
#else
		auto result = -1;
#endif
		co_return result == 0;
	});
	mailHub->create("daemon", daemonService);
}

void DaemonServer::onExit()
{
	ClusterServer::onExit();
}