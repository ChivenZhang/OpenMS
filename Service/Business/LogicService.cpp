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
		co_return co_await this->onLoginRequest(user, pass);
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
	this->bind("onBattleMatch", [this](uint32_t gameID, MSList<uint32_t> userIDs)->MSAsync<void>
	{
		co_return co_await this->onBattleMatch(gameID, userIDs);
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

	this->bind("onSpaceCreate", [this](uint32_t spaceID)->MSAsync<void>
	{
		MS_INFO("空间回调 %u", spaceID);
		co_return co_await this->onSpaceCreate(spaceID);
	});

	this->bind("onSpaceEnter", [this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
	{
		MS_INFO("空间进入回调 %u", spaceID);
		co_return co_await this->onSpaceEnter(spaceID, userID);
	});

	this->bind("onSpaceLeave", [this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
	{
		MS_INFO("空间离开回调 %u", spaceID);
		co_return co_await this->onSpaceLeave(spaceID, userID);
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

MSAsync<uint32_t> LogicService::onLoginRequest(MSString username, MSString password)
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

	MSMutexLock lock(m_UserLock);
	auto& userInfo = m_UserInfos[userID];
	userInfo.SpaceID = 0;
	userInfo.Online = true;
	userInfo.InGame = false;
	userInfo.LastUpdate = ::clock() * 1.0f / CLOCKS_PER_SEC;
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

MSAsync<void> LogicService::onBattleMatch(uint32_t gameID, MSList<uint32_t> userIDs)
{
	if (auto spaceID = co_await this->onSpaceRequest(gameID))
	{
		MSMutexLock lock(m_UserLock);
		auto& space = m_SpaceInfos[spaceID];
		space.GameID = gameID;
		space.UserIDs = userIDs;
	}
	co_return;
}

MSAsync<uint32_t> LogicService::onSpaceRequest(uint32_t gameID)
{
	MS_INFO("空间请求");
	static uint32_t s_SpaceID = 0;
	auto spaceID = ++s_SpaceID;
	co_await this->async<bool>("daemon", "createSpace", "", 0, MSTuple{ this->name(), spaceID, gameID});
	co_return spaceID;
}

MSAsync<void> LogicService::onSpaceCreate(uint32_t spaceID)
{
	m_UserLock.lock();
	auto space = m_SpaceInfos[spaceID];
	m_UserLock.unlock();
	for (auto& userID : space.UserIDs)
	{
		co_await this->async<bool>("space:" + std::to_string(spaceID), "enterSpace", "", 100, MSTuple{userID});
	}
	co_return;
}

MSAsync<void> LogicService::onSpaceDelete(uint32_t spaceID)
{
	co_return;
}

MSAsync<void> LogicService::onSpaceEnter(uint32_t spaceID, uint32_t userID)
{
	m_UserLock.lock();
	auto& userInfo = m_UserInfos[userID];
	userInfo.SpaceID = spaceID;
	userInfo.InGame = true;
	m_UserLock.unlock();
	co_return;
}

MSAsync<void> LogicService::onSpaceLeave(uint32_t spaceID, uint32_t userID)
{
	m_UserLock.lock();
	auto& userInfo = m_UserInfos[userID];
	userInfo.SpaceID = spaceID;
	userInfo.InGame = false;
	m_UserLock.unlock();
	co_return;
}