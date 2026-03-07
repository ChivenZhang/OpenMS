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
	m_GameID(gameID),
	m_GameState(state_t::NONE)
{
	this->bind("beginPlay", [=, this]()->MSAsync<void>
	{
		m_GameState = state_t::START;
		for (auto& user : m_UserInfos)
		{
			auto userID = user.first;
			co_await this->callPlayer<void>(userID, "startBattle", 0, MSTuple{});
		}
		co_return;
	});
	this->bind("endPlay", [=, this]()->MSAsync<void>
	{
		m_GameState = state_t::STOP;
		for (auto& user : m_UserInfos)
		{
			auto userID = user.first;
			co_await this->callPlayer<void>(userID, "stopBattle", 0, MSTuple{});
		}
		AUTOWIRE_DATA(IServer)->shutdown();
		co_return;
	});
	this->bind("enterSpace", [=, this](MSString caller, uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("用户 %u 加入空间", userID);

		auto playerService = this->onCreatingPlayer(userID);
		if (this->create("player:" + std::to_string(userID), playerService))
		{
			auto& user = m_UserInfos[userID];
			user.Player = playerService;
			co_await playerService->onCreatePlayer();
			co_await this->async<void>(caller, "onEnterSpace", "", 0, MSTuple{m_SpaceID, userID });
			co_return true;
		}
		co_return false;
	});
	this->bind("reenterSpace", [=, this](MSString caller, uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("用户 %u 重新加入空间", userID);

		if (this->exist("player:" + std::to_string(userID)))
		{
			auto& user = m_UserInfos[userID];
			co_await this->async<void>(caller, "onEnterSpace", "", 0, MSTuple{m_SpaceID, userID });
			// Whole synchronization of game state
			co_await this->async<void>(this->name(), "syncWhole", "", 0, MSTuple{userID});
			co_return true;
		}
		co_return false;
	});
	this->bind("leaveSpace", [=, this](uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("用户 %u 离开空间", userID);

		if (this->cancel("player:" + std::to_string(userID)))
		{
			m_UserInfos.erase(userID);
			co_await this->callPlayer<void>(userID, "onLeaveSpace", 0, MSTuple{m_SpaceID, userID});
		}
		co_return false;
	});
	this->bind("syncWhole", [=, this]()->MSAsync<void>
	{
		if(m_GameState == state_t::START)
		{
			for (auto& user : m_UserInfos)
			{
				auto userID = user.first;
				this->callClient(userID, "onStateChange", 0, MSTuple{"{name:value}"}, [](){});
			}
		}
		co_return;
	});
	this->bind("syncDelta", [=, this]()->MSAsync<void>
	{
		if(m_GameState == state_t::START)
		{
			// TODO: A thousand years later
		}
		co_return;
	});

	m_Timer.start(1000, 33, [=, this]()
	{
		this->call<void>(this->name(), "syncDelta", "", 0, MSTuple{});
	});

	m_Timer.start(1000, 2000, [=, this]()
	{
		this->call<void>(this->name(), "syncWhole", "", 0, MSTuple{});
	});
}

MSRef<PlayerService> SpaceService::onCreatingPlayer(uint32_t userID)
{
	return MSNew<PlayerService>(userID);
}

uint32_t SpaceService::spaceID() const
{
    return m_SpaceID;
}

uint32_t SpaceService::gameID() const
{
	return m_GameID;
}