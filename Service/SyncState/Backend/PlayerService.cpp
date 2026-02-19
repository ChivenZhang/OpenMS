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
#include "PlayerService.h"

PlayerService::PlayerService(uint32_t userID)
	:
	m_UserID(userID)
{
	this->bind("startBattle", [=, this]()->MSAsync<void>
	{
		MS_INFO("用户 %u 开始游戏", userID);
		co_await this->callClient<void>("onStartBattle", 0, MSTuple{});
		co_return;
	});
	this->bind("stopBattle", [=, this]()->MSAsync<void>
	{
		MS_INFO("用户 %u 结束游戏", userID);
		co_await this->callClient<void>("onStopBattle", 0, MSTuple{});
		co_return;
	});
}

uint32_t PlayerService::userID() const
{
	return m_UserID;
}

MSAsync<void> PlayerService::onCreatePlayer()
{
	MS_INFO("创建玩家 %u", m_UserID);
	co_return;
}