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
#include <OpenMS/Server/IServer.h>
#include <gflags/gflags_declare.h>
DECLARE_string(caller);

namespace state_server
{
	SpaceService::SpaceService(uint32_t spaceID, uint32_t gameID)
		:
		m_SpaceID(spaceID),
		m_GameID(gameID),
		m_GameState(state_t::NONE)
	{
		this->bind("enterSpace", [=, this](uint32_t userID)->MSAsync<bool>
		{
			MS_INFO("用户 {} 加入空间", userID);

			auto playerService = this->onCreatingPlayer(userID);
			if (this->create("player:" + std::to_string(userID), playerService))
			{
				auto& user = m_UserInfos[userID];
				user.Player = playerService;
				co_await playerService->onCreatePlayer();
				co_await this->onEnterSpace(userID);
				co_await this->async<void>(FLAGS_caller, "onEnterSpace", "", 0, MSTuple{ m_SpaceID, userID });
				co_return true;
			}
			co_return false;
		});
		this->bind("reenterSpace", [=, this](uint32_t userID)->MSAsync<bool>
		{
			MS_INFO("用户 {} 重新加入空间", userID);

			if (this->exist("player:" + std::to_string(userID)))
			{
				auto& user = m_UserInfos[userID];
				co_await this->onEnterSpace(userID);
				co_await this->async<void>(FLAGS_caller, "onEnterSpace", "", 0, MSTuple{ m_SpaceID, userID });
				// Whole synchronization of game state
				co_await this->async<void>(this->name(), "syncFull", "", 0, MSTuple{userID});
				co_return true;
			}
			co_return false;
		});
		this->bind("leaveSpace", [=, this](uint32_t userID)->MSAsync<bool>
		{
			MS_INFO("用户 {} 离开空间", userID);

			auto playerService = m_UserInfos.emplace(userID, user_t{}).first->second.Player.lock();
			if (this->cancel("player:" + std::to_string(userID)))
			{
				m_UserInfos.erase(userID);
				co_await this->onLeaveSpace(userID);
				co_await playerService->onDeletePlayer();
				co_await this->async<void>(FLAGS_caller, "onLeaveSpace", "", 0, MSTuple{m_SpaceID, userID });
			}
			co_return false;
		});
		this->bind("beginPlay", [=, this]()->MSAsync<void>
		{
			m_GameState = state_t::START;
			for (auto& user : m_UserInfos)
			{
				auto userID = user.first;
				co_await this->callPlayer<void>(userID, "onStartBattle", 0, MSTuple{});
			}
			co_return;
		});
		this->bind("endPlay", [=, this]()->MSAsync<void>
		{
			m_GameState = state_t::STOP;

			MSList<uint32_t> userIDs;
			for (auto& user : m_UserInfos) userIDs.push_back(user.first);

			for (auto userID : userIDs)
			{
				co_await this->callPlayer<void>(userID, "onStopBattle", 0, MSTuple{});
			}
			for (auto userID : userIDs)
			{
				co_await this->async<void>(this->name(), "leaveSpace", "", 0, MSTuple{ userID });
			}
			AUTOWIRE_DATA(IServer)->shutdown();
			co_return;
		});
		this->bind("syncFull", [=, this]()->MSAsync<void>
		{
			if(m_GameState == state_t::START)
			{
				for (auto& user : m_UserInfos)
				{
					MSString state;
					auto userID = user.first;
					auto player = user.second.Player.lock();
					player->onStateChange(state, true);
					for (auto& peer : m_UserInfos)
					{
						auto peerID = peer.first;
						MS_INFO("sync {}", userID);
						co_await this->async<void>("client", "onStateChange", "proxy:" + std::to_string(peerID), 0, MSTuple{userID, state, true});
					}
				}
			}
			co_return;
		});
		this->bind("syncDelta", [=, this]()->MSAsync<void>
		{
			if(m_GameState == state_t::START)
			{
				for (auto& user : m_UserInfos)
				{
					MSString state;
					auto userID = user.first;
					auto player = user.second.Player.lock();
					player->onStateChange(state, false);
					for (auto& peer : m_UserInfos)
					{
						auto peerID = peer.first;
						co_await this->async<void>("client", "onStateChange", "proxy:" + std::to_string(peerID), 0, MSTuple{userID, state, false});
					}
				}
			}
			co_return;
		});

		m_Timer.start(1000, 33, [=, this]()
		{
			this->call<void>(this->name(), "syncDelta", "", 0, MSTuple{});
		});

		m_Timer.start(1000, 2000, [=, this]()
		{
			this->call<void>(this->name(), "syncFull", "", 0, MSTuple{});
		});
	}

	MSRef<PlayerService> SpaceService::onCreatingPlayer(uint32_t userID)
	{
		return MSNew<PlayerService>(userID);
	}

	MSAsync<void> SpaceService::onEnterSpace(uint32_t userID)
	{
		co_return;
	}

	MSAsync<void> SpaceService::onLeaveSpace(uint32_t userID)
	{
		co_return;
	}

	uint32_t SpaceService::spaceID() const
	{
		return m_SpaceID;
	}

	uint32_t SpaceService::gameID() const
	{
		return m_GameID;
	}
}