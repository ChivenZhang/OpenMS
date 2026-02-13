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
#include "LogicService.h"
#include "ServerService.h"

LogicService::LogicService()
{
	this->bind("login", [this](MSString user, MSString pass)->MSAsync<uint32_t>
	{
		MS_INFO("登录请求");
		co_return co_await this->onRequestLogin(user, pass);
	});
	this->bind("onClientLogin", [this](uint32_t userID, uint32_t code, MSString error)->MSAsync<void>
	{
		MS_INFO("登录回调");
		co_return co_await this->onClientLogin(userID, code, error);
	});
	this->bind("signup", [this](MSString user, MSString pass)->MSAsync<uint32_t>
	{
		MS_INFO("注册请求");
		co_return co_await this->onSignupRequest(user, pass);
	});
	this->bind("onClientSignup", [this](uint32_t userID, uint32_t code, MSString error)->MSAsync<void>
	{
		MS_INFO("注册回调");
		co_return co_await this->onClientSignup(userID, code, error);
	});
	this->bind("logout", [=, this](uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("注销请求");	// TODO: 心跳超时，自动登出
		auto result = false;
		result |= this->cancel("server:" + std::to_string(userID));
		result |= this->cancel("player:" + std::to_string(userID));

		if (result)
		{
			MSMutexLock lock(m_UserLock);
			auto& userInfo = m_UserInfos[userID];
			userInfo.Online = false;
		}
		co_return true;
	});

	// Match Module

	this->bind("matchBattle", [this](uint32_t userID, uint32_t gameID)->MSAsync<void>
	{
		co_return co_await this->async<void>("match", "enterMatch", "", 0, MSTuple{userID, gameID});
	});
	this->bind("unmatchBattle", [this](uint32_t userID)->MSAsync<void>
	{
		co_return co_await this->async<void>("match", "leaveMatch", "", 0, MSTuple{userID});
	});
	this->bind("onMatchBattle", [this](uint32_t gameID, MSList<uint32_t> userIDs)->MSAsync<void>
	{
		co_return co_await this->onMatchBattle(gameID, userIDs);
	});
	this->bind("updateMatch", [this]()->MSAsync<void>
	{
		MSList<uint32_t> spaceIDs;
		m_UserLock.lock();
		for (auto& space : m_SpaceInfos)
		{
			if (space.second.InGame) continue;
			auto inGame = true;
			for (auto& userID : space.second.UserIDs)
			{
				inGame &= m_UserInfos[userID].InGame;
			}
			space.second.InGame = inGame;
			if (inGame) spaceIDs.push_back(space.first);
		}
		for (auto& spaceID : spaceIDs)
		{
			m_SpaceInfos.erase(spaceID);
		}
		m_UserLock.unlock();

		for (auto spaceID : spaceIDs)
		{
			co_await this->async<void>("space:" + std::to_string(spaceID), "beginPlay", "", 0, MSTuple{});
		}
		co_return;
	});

	// Space Module

	this->bind("onCreateSpace", [this](uint32_t spaceID)->MSAsync<void>
	{
		MS_INFO("创建空间回调 %u", spaceID);
		co_return co_await this->onCreateSpace(spaceID);
	});

	this->bind("onDeleteSpace", [this](uint32_t spaceID)->MSAsync<void>
	{
		MS_INFO("删除空间回调 %u", spaceID);
		co_return co_await this->onDeleteSpace(spaceID);
	});

	this->bind("onEnterSpace", [this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
	{
		MS_INFO("进入空间回调 %u", spaceID);
		co_await this->onEnterSpace(spaceID, userID);

		// Broadcast to peers in the space

		MSMutexLock lock(m_UserLock);
		auto& space = m_SpaceInfos[spaceID];
		for (auto peerID : space.UserIDs)
		{
			this->async<void>("client", "onEnterSpace", "proxy:" + std::to_string(peerID), 0, MSTuple{spaceID, userID});
		}
		co_return;
	});

	this->bind("onLeaveSpace", [this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
	{
		MS_INFO("离开空间回调 %u", spaceID);
		co_await this->onLeaveSpace(spaceID, userID);

		// Broadcast to peers in the space

		MSMutexLock lock(m_UserLock);
		auto& space = m_SpaceInfos[spaceID];
		for (auto peerID : space.UserIDs)
		{
			this->async<void>("client", "onLeaveSpace", "proxy:" + std::to_string(peerID), 0, MSTuple{spaceID, userID});
		}
		co_return;
	});
}

MSAsync<MSString> LogicService::globalData(MSStringView name)
{
	co_return "";
}

MSAsync<void> LogicService::globalData(MSStringView name, MSStringView value)
{
	co_return;
}

MSAsync<void> LogicService::onGlobalData(MSStringView name, MSStringView value)
{
	co_return;
}

MSAsync<uint32_t> LogicService::onRequestLogin(MSString username, MSString password)
{
	co_return co_await this->async<uint32_t>("login", "login", "", 100, MSTuple{this->name(), username, password});
}

MSAsync<void> LogicService::onClientLogin(uint32_t userID, uint32_t code, MSString error)
{
	auto serverService = MSNew<ServerService>(userID);
	serverService->bind("matchBattle", [=, this](uint32_t gameID)->MSAsync<bool>
	{
		this->call<void>("logic", "matchBattle", "", 0, MSTuple{userID, gameID});
		co_return true;
	});
	serverService->bind("unmatchBattle", [=, this]()->MSAsync<bool>
	{
		this->call<void>("logic", "unmatchBattle", "", 0, MSTuple{userID});
		co_return true;
	});
	serverService->bind("notifyBattle", [=, this]()->MSAsync<void>
	{
		MSMutexLock lock(m_UserLock);
		auto result = m_UserInfos.find(userID);
		if (result != m_UserInfos.end() && result->second.Online)
		{
			auto& userInfo = result->second;
			userInfo.LastUpdate = ::clock() * 1.0f / CLOCKS_PER_SEC;
		}
		co_return;
	});
	this->create("server:" + std::to_string(userID), serverService);

	// Update record

	m_UserLock.lock();
	auto result = m_UserInfos.emplace(userID, UserInfo{});
	if(result.second)
	{
		// New user, initialize info
		auto& userInfo = result.first;
		userInfo.SpaceID = 0;
		userInfo.Online = true;
		userInfo.InGame = false;
		userInfo.LastUpdate = ::clock() * 1.0f / CLOCKS_PER_SEC;
		m_UserLock.unlock();
	}
	else
	{
		// Existing user, update info
		auto& userInfo = result.first;
		userInfo.Online = true;
		userInfo.LastUpdate = ::clock() * 1.0f / CLOCKS_PER_SEC;
		if(userInfo.InGame && userInfo.SpaceID)
		{
			auto spaceID = userInfo.SpaceID;
			m_UserLock.unlock();
			co_await this->async<void>("space:" + std::to_string(spaceID), "reenterSpace", "", 0, MSTuple{this->name(), userID});
		}
		else
		{
			// Not in game, treat as normal login
			userInfo.SpaceID = 0;
			userInfo.InGame = false;
			m_UserLock.unlock();
		}
	}
	co_return;
}

MSAsync<uint32_t> LogicService::onSignupRequest(MSString username, MSString password)
{
	co_return co_await this->async<uint32_t>("login", "signup", "", 100, MSTuple{this->name(), username, password});
}

MSAsync<void> LogicService::onClientSignup(uint32_t userID, uint32_t code, MSString error)
{
	co_return;
}

MSAsync<void> LogicService::onMatchBattle(uint32_t gameID, MSList<uint32_t> userIDs)
{
	if (auto spaceID = co_await this->onRequestSpace(gameID))
	{
		MSMutexLock lock(m_UserLock);
		auto& space = m_SpaceInfos[spaceID];
		space.GameID = gameID;
		space.UserIDs = userIDs;
	}
	co_return;
}

MSAsync<uint32_t> LogicService::onRequestSpace(uint32_t gameID)
{
	MS_INFO("空间请求");
	static uint32_t s_SpaceID = 0;
	auto spaceID = ++s_SpaceID;
	co_await this->async<bool>("daemon", "createSpace", "", 0, MSTuple{ this->name(), spaceID, gameID});
	co_return spaceID;
}

MSAsync<void> LogicService::onCreateSpace(uint32_t spaceID)
{
	m_UserLock.lock();
	auto space = m_SpaceInfos[spaceID];
	m_UserLock.unlock();
	for (auto& userID : space.UserIDs)
	{
		co_await this->async<void>("space:" + std::to_string(spaceID), "enterSpace", "", 0, MSTuple{ this->name(), userID});
	}
	co_return;
}

MSAsync<void> LogicService::onDeleteSpace(uint32_t spaceID)
{
	co_return;
}

MSAsync<void> LogicService::onEnterSpace(uint32_t spaceID, uint32_t userID)
{
	MSMutexLock lock(m_UserLock);
	auto& userInfo = m_UserInfos[userID];
	userInfo.SpaceID = spaceID;
	userInfo.InGame = true;
	co_return;
}

MSAsync<void> LogicService::onLeaveSpace(uint32_t spaceID, uint32_t userID)
{
	MSMutexLock lock(m_UserLock);
	auto& userInfo = m_UserInfos[userID];
	userInfo.SpaceID = spaceID;
	userInfo.InGame = false;
	co_return;
}