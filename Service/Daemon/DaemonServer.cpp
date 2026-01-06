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
		auto result = system("start notepad.exe");
		co_return result == 0;
	});
	mailHub->create("daemon", daemonService);
}

void DaemonServer::onExit()
{
	ClusterServer::onExit();
}