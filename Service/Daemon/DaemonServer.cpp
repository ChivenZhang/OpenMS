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
	auto launchName = property<MSString>(identity() + ".launch", "BackendServer");
	auto launchPath = (std::filesystem::absolute(IStartup::Argv[0]).parent_path() / launchName).generic_string();
	
	auto daemonService = MSNew<Service>();

	daemonService->bind("createSpace", [=](MSString caller, uint32_t spaceID, uint32_t gameID)->MSAsync<bool>
	{
		MS_INFO("守护进程：CREATE SPACE!!! %u", spaceID);
#ifdef OPENMS_PLATFORM_WINDOWS
		auto result = system(("start " + launchPath + ".exe" " --space=" + std::to_string(spaceID) + " --game=" + std::to_string(gameID) + " --caller=" + caller + " >> " + std::to_string(spaceID) + ".log 2>&1").c_str());
#elif defined(OPENMS_PLATFORM_APPLE)
		auto result = system(("nohup " + launchPath + " --space=" + std::to_string(spaceID) + " --game=" + std::to_string(gameID) + " --caller=" + caller + " >> " + std::to_string(spaceID) + ".log &").c_str());
#elif defined(OPENMS_PLATFORM_LINUX)
		auto result = system(("nohup " + launchPath + " --space=" + std::to_string(spaceID) + " --game=" + std::to_string(gameID) + " --caller=" + caller + " >> " + std::to_string(spaceID) + ".log &").c_str());
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