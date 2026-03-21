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
#include <OpenMS/Server/IServer.h>

PlayerService::PlayerService(uint32_t userID)
	:
	m_UserID(userID)
{
	this->bind("onStartBattle", [=, this]()->MSAsync<void>
	{
		co_return co_await this->onStartBattle();
	});
	this->bind("onStopBattle", [=, this]()->MSAsync<void>
	{
		co_return co_await this->onStopBattle();
	});
	this->bind("onAttack", [=]()->MSAsync<void>
	{
		MS_INFO("发动攻击");
		co_return;
	});
}

MSAsync<void> PlayerService::onStartBattle()
{
	MS_INFO("开始游戏");
	co_return co_await this->callPlayer<void>("attack", 0, MSTuple{});
}

MSAsync<void> PlayerService::onStopBattle()
{
	MS_INFO("结束游戏");
	AUTOWIRE_DATA(IServer)->shutdown();
	co_return;
}