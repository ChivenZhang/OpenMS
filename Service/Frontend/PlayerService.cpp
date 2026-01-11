/*=================================================
* Copyright © 2020-2026 ChivenZhang.
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
	this->bind("onStartBattle", [=, this](uint32_t spaceID)->MSAsync<void>
	{
		co_await this->callPlayer<bool>("attack", 100, MSTuple{});

		MS_INFO("开始游戏");
		co_return co_await this->onStartBattle(spaceID);
	});
	this->bind("onStopBattle", [=, this]()->MSAsync<void>
	{
		MS_INFO("结束游戏");
		co_return co_await this->onStopBattle();
	});
	this->bind("onAttack", [=]()->MSAsync<void>
	{
		MS_INFO("发动攻击");
		co_return;
	});
}

MSAsync<void> PlayerService::onStartBattle(uint32_t spaceID)
{
	co_return;
}

MSAsync<void> PlayerService::onStopBattle()
{
	co_return;
}