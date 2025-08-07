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
#include "DiscoveryService.h"

// ========================================================================================

MSString DiscoveryService::identity() const
{
	return "discovery";
}

void DiscoveryService::onInit()
{
	MasterService::onInit();
}

void DiscoveryService::onExit()
{
	MasterService::onExit();
}