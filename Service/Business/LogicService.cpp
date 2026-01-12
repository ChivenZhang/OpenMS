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

	this->bind("matchBattle", [this]()->MSAsync<void>
	{
		MS_INFO("服务端：START BATTLE!!!");	// Assume battle ready

		co_await this->createSpace();
		co_return;
	});

	this->bind("onSpaceCreate", [this](uint32_t spaceID)->MSAsync<void>
	{
		MS_INFO("空间回调 %u", spaceID);
		co_return co_await this->onSpaceCreate(spaceID);
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
	auto serverService = MSNew<ServerService>();
	serverService->bind("readyBattle", [=, this](uint32_t gameID)->MSAsync<bool>
	{
		MS_INFO("服务端：READY BATTLE!!!");

		MSMutexLock lock(m_UserLock);
		m_MatchQueue.push(userID);
		this->call<void>("logic", "matchBattle", "", 0, MSTuple{});
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

MSAsync<uint32_t> LogicService::createSpace()
{
	MS_INFO("空间请求");
	static uint32_t s_SpaceID = 0;
	auto spaceID = ++s_SpaceID;
	auto result = co_await this->async<bool>("daemon", "createSpace", "", 100, MSTuple{ this->name(), spaceID });
	co_return result ? spaceID : 0U;
}

MSAsync<bool> LogicService::deleteSpace(uint32_t spaceID)
{
	co_return co_await this->async<bool>("daemon", "deleteSpace", "", 100, MSTuple{ spaceID });
}

MSAsync<void> LogicService::onSpaceCreate(uint32_t spaceID)
{
	if (spaceID)
	{
		MSMutexLock lock(m_UserLock);
		while (m_MatchQueue.empty() == false)
		{
			auto userID = m_MatchQueue.front();
			m_MatchQueue.pop();

			auto& userInfo = m_UserInfos[userID];
			userInfo.SpaceID = spaceID;

			if (co_await this->async<bool>("space:" + std::to_string(spaceID), "enterSpace", "", 100, MSTuple{userID}))
			{
				co_await this->async<bool>("player:" + std::to_string(userID), "startBattle", "", 0, MSTuple{});
			}
		}
	}
	co_return;
}

MSAsync<void> LogicService::onSpaceDelete(uint32_t spaceID)
{
	co_return;
}
