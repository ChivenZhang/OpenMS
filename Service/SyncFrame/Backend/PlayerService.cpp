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
	m_UserID(userID),
	m_TickTime(0.0f)
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
	this->bind("keepAlive", [=, this]()->MSAsync<void>
	{
		m_TickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() * 0.001f;
		co_return;
	});
	this->bind("attack", [=, this]()->MSAsync<void>
	{
		MS_INFO("用户 %u 发动攻击", userID);
		co_await this->callClient<void>("onAttack", 0, MSTuple{});
		co_return;
	});
}

float PlayerService::tickTime() const
{
    return m_TickTime;
}
