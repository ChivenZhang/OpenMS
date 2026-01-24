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

	m_ClusterServer->bind("guestID", [](MSHnd<IChannel> client)->uint32_t
	{
		static uint32_t s_GuestID = 0;
		return ++s_GuestID;
	});

	m_ClusterServer->bind("spaceID", [](MSHnd<IChannel> client)->uint32_t
	{
		static uint32_t s_SpaceID = 0;
		return ++s_SpaceID;
	});
}

void DiscoveryServer::onExit()
{
	MasterServer::onExit();
}