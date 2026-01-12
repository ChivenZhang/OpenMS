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
{
	this->bind("startBattle", [=, this]()->MSAsync<bool>
	{
		MS_INFO("用户 %u 开始游戏", userID);
		this->call<void>("client:" + std::to_string(userID), "onStartBattle", "proxy:" + std::to_string(userID), 0, MSTuple{});
		co_return true;
	});
	this->bind("stopBattle", [=, this]()->MSAsync<bool>
	{
		MS_INFO("用户 %u 结束游戏", userID);
		this->call<void>("client:" + std::to_string(userID), "onStopBattle", "proxy:" + std::to_string(userID), 0, MSTuple{});
		co_return true;
	});
	this->bind("attack", [=, this]()->MSAsync<void>
	{
		MS_INFO("用户 %u 发动攻击", userID);
		this->call<void>("client:" + std::to_string(userID), "onAttack", "proxy:" + std::to_string(userID), 0, MSTuple{});
		co_return;
	});
}