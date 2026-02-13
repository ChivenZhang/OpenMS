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

#include "iocpp.h"
#include "PlayerService.h"
#include "Server/IServer.h"

SpaceService::SpaceService(uint32_t spaceID, uint32_t gameID)
	:
	m_SpaceID(spaceID),
	m_GameID(gameID)
{
	this->bind("beginPlay", [=, this]()->MSAsync<void>
	{
		for (auto& user : m_UserInfos)
		{
			auto userID = user.second.UserID;
			co_await this->async<void>("player:" + std::to_string(userID), "startBattle", "", 0, MSTuple{});
		}
		co_return;
	});
	this->bind("endPlay", [=, this]()->MSAsync<void>
	{
		for (auto& user : m_UserInfos)
		{
			auto userID = user.second.UserID;
			co_await this->async<void>("player:" + std::to_string(userID), "stopBattle", "", 0, MSTuple{});
		}
		AUTOWIRE_DATA(IServer)->shutdown();
		co_return;
	});
	this->bind("enterSpace", [=, this](MSString caller, uint32_t userID)->MSAsync<void>
	{
		MS_INFO("用户 %u 加入空间", userID);

		if (this->create("player:" + std::to_string(userID), MSNew<PlayerService>(userID)))
		{
			auto& user = m_UserInfos[userID];
			user.UserID = userID;
			co_await this->async<void>(caller, "onEnterSpace", "", 0, MSTuple{m_SpaceID, userID });
		}
		co_return;
	});
	this->bind("reenterSpace", [=, this](MSString caller, uint32_t userID)->MSAsync<void>
	{
		MS_INFO("用户 %u 重新加入空间", userID);

		if (this->exists("player:" + std::to_string(userID)))
		{
			auto& user = m_UserInfos[userID];
			user.UserID = userID;
			co_await this->async<void>(caller, "onEnterSpace", "", 0, MSTuple{m_SpaceID, userID });

			// Entail synchronization of game state

			co_await this->async<void>(this->name(), "syncWhole", "", 0, MSTuple{userID});
		}
		co_return;
	});
	this->bind("leaveSpace", [=, this](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("用户 %u 离开空间", userID);

		if (this->cancel("player:" + std::to_string(userID)))
		{
			m_UserInfos.erase(userID);
			co_await this->async<void>("player:" + std::to_string(userID), "onLeaveSpace", "", 0, MSTuple{m_SpaceID, userID});
		}
		co_return;
	});
	this->bind("keepAlive", [=](uint32_t userID)->MSAsync<void>
	{
		co_return;
	});
	this->bind("checkAlive", [=]()->MSAsync<void>
	{
		co_return;
	});
	this->bind("syncState", [=](uint32_t userID)->MSAsync<void>
	{
		// TODO: A thousand years later
		co_return;
	});
	this->bind("syncWhole", [=](uint32_t userID)->MSAsync<void>
	{
		// TODO: A thousand years later
		co_return;
	});

	m_Timer.start(1000, 1000, [=, this]()
	{
		for (auto& user : m_UserInfos)
		{
			auto userID = user.second.UserID;
			this->call<void>("client:" + std::to_string(userID), "onStateChange", "proxy:" + std::to_string(userID), 0, MSTuple{"{name:value}"});
		}
	});
}