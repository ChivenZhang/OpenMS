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
#include "GatewayServer.h"

// ========================================================================================

MSString GatewayServer::identity() const
{
	return "discovery";
}

void GatewayServer::onInit()
{
	MasterServer::onInit();
}

void GatewayServer::onExit()
{
	MasterServer::onExit();
}