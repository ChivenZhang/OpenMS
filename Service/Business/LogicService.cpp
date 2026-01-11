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
	this->bind("login", [=, this](MSString user, MSString pass)->MSAsync<uint32_t>
	{
		MS_INFO("服务端：LOGIN!!!");
		auto userID = co_await this->onLoginRequest(user, pass);

		{
			MSMutexLock lock(m_UserLock);
			auto& userInfo = m_UserInfos[userID];
			userInfo.SpaceID = 0;
			userInfo.Online = true;
			userInfo.LastUpdate = ::clock() * 1.0f / CLOCKS_PER_SEC;
		}

		auto serverService = MSNew<ServerService>();
		serverService->bind("readyBattle", [=, this](uint32_t gameID)->MSAsync<bool>
		{
			MS_INFO("服务端：READY BATTLE!!!");

			// Match battle
			this->call<bool>("logic", "matchBattle", "", 0, MSTuple{userID});

			co_return true;
		});
		serverService->bind("notifyBattle", [=, this]()->MSAsync<void>
		{
			MSMutexLock lock(m_BattleLock);
			auto result = m_UserInfos.find(userID);
			if (result != m_UserInfos.end() && result->second.Online)
			{
				auto& userInfo = result->second;
				userInfo.LastUpdate = ::clock() * 1.0f / CLOCKS_PER_SEC;
			}
			co_return;
		});
		this->create("server:" + std::to_string(userID), serverService);
		co_return userID;
	});
	this->bind("logout", [=, this](uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("服务端：LOGOUT!!!");	// TODO: 心跳超时，自动登出
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
	this->bind("signup", [](MSString user, MSString pass)->MSAsync<bool>
	{
		MS_INFO("服务端：SIGNUP!!!");
		co_return false;
	});

	// Match Module

	this->bind("matchBattle", [this](uint32_t userID)->MSAsync<uint32_t>
	{
		MS_INFO("服务端：START BATTLE!!!");	// Assume battle ready

		auto spaceID = ++m_SpaceID;
		if (co_await this->async<bool>("daemon", "createSpace", "", 100, MSTuple{ spaceID }))
		{
			MSMutexLock lock(m_UserLock);
			auto& userInfo = m_UserInfos[userID];
			userInfo.SpaceID = spaceID;
			this->call<bool>("player:" + std::to_string(userID), "startBattle", "", 0, MSTuple{});
		}
		co_return spaceID;
	});
	this->bind("onCreateSpace", [=](uint32_t spaceID)->MSAsync<void>
	{
		co_return;
	});
}

MSAsync<uint32_t> LogicService::onLoginRequest(MSStringView username, MSStringView password)
{
	static uint32_t s_UserID = 0;
	co_return ++s_UserID;
}

MSAsync<uint32_t> LogicService::onCreateSpace()
{
	static uint32_t s_SpaceID = 0;
	auto spaceID = ++s_SpaceID;
	auto result = co_await this->async<bool>("daemon", "createSpace", "", 100, MSTuple{ spaceID });
	co_return result ? spaceID : 0U;
}

MSAsync<bool> LogicService::onDeleteSpace(uint32_t spaceID)
{
	co_return co_await this->async<bool>("daemon", "deleteSpace", "", 100, MSTuple{ spaceID });
}