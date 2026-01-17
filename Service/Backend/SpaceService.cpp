/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "SpaceService.h"
#include "PlayerService.h"

SpaceService::SpaceService(uint32_t spaceID)
	:
	m_SpaceID(spaceID)
{
	this->bind("enterSpace", [=, this](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("用户 %u 加入空间", userID);

		this->create("player:" + std::to_string(userID), MSNew<PlayerService>(userID));

		co_return co_await this->async<void>("logic", "onSpaceEnter", "", 0, MSTuple{m_SpaceID, userID});
	});
	this->bind("leaveSpace", [=, this](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("用户 %u 离开空间", userID);

		this->cancel("player:" + std::to_string(userID));

		co_return co_await this->async<void>("logic", "onSpaceLeave", "", 0, MSTuple{m_SpaceID, userID});
	});
	this->bind("keepAlive", [=](uint32_t userID)->MSAsync<void>
	{
		co_return;
	});
	this->bind("checkAlive", [=]()->MSAsync<void>
	{
		co_return;
	});
	this->bind("syncStatus", [=](uint32_t userID)->MSAsync<void>
	{
		co_return;
	});
}
