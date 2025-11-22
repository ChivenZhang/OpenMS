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
#include "DiscoveryServer.h"

// ========================================================================================

MSString DiscoveryServer::identity() const
{
	return "discovery";
}

void DiscoveryServer::onInit()
{
	MasterServer::onInit();
}

void DiscoveryServer::onExit()
{
	MasterServer::onExit();
}