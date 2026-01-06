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
#include <OpenMS/Mailbox/Private/Mail.h>
#include <OpenMS/Server/Private/Service.h>

MSString BackendServer::identity() const
{
	return "backend";
}

void BackendServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();
}

void BackendServer::onExit()
{
	ClusterServer::onExit();
}