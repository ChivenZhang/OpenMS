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
	this->bind("attack", [=, this]()->MSAsync<void>
	{
		MS_INFO("用户 %u 发动攻击", userID);
		co_await this->callClient<void>("onAttack", 0, MSTuple{});
		co_return;
	});
}