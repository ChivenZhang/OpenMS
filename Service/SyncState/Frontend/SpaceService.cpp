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
#include "SpaceService.h"

SpaceService::SpaceService()
{
	this->bind("onLogin", [=, this](uint32_t userID)->MSAsync<void>
	{
		co_return co_await this->onLogin(userID);
	});
	this->bind("onLogout", [=, this](uint32_t userID, bool result)->MSAsync<void>
	{
		co_return co_await this->onLogout(userID, result);
	});
	this->bind("onSignup", [=, this](bool result)->MSAsync<void>
	{
		co_return co_await this->onSignup(result);
	});
	this->bind("onEnterSpace", [=, this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
	{
		if(this->exist("client:" + std::to_string(userID)) == false)
		{
			auto playerService = this->onCreatingPlayer(userID);
			this->create("client:" + std::to_string(userID), playerService);
		}
		co_return co_await this->onEnterSpace(spaceID, userID);
	});
	this->bind("onLeaveSpace", [=, this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
	{
		if(this->exist("client:" + std::to_string(userID)))
		{
			this->cancel("client:" + std::to_string(userID));
		}
		co_return co_await this->onLeaveSpace(spaceID, userID);
	});
}

uint32_t SpaceService::userID() const
{
    return m_UserID;
}

MSRef<PlayerService> SpaceService::player() const
{
    return m_PlayerService.lock();
}

bool SpaceService::signup(MSString username, MSString password, uint32_t timeout)
{
	return this->async("guest", "guest", "", 100, MSTuple{}, [=, this](uint32_t guestID)
	{
		if (guestID == 0) return;
		auto guestName = "guest:" + std::to_string(guestID);

		MS_INFO("正在注册...");

		this->async(guestName, "signup", "", timeout, MSTuple{username, password}, [](bool result)
		{
			MS_INFO("注册结果：%s", result ? "true" : "false");
		});
	});
}

bool SpaceService::login(MSString username, MSString password, uint32_t timeout)
{
	return this->async("guest", "guest", "", 100, MSTuple{}, [=, this](uint32_t guestID)
	{
		if (guestID == 0) return;
		auto guestName = "guest:" + std::to_string(guestID);

		MS_INFO("正在登录...");

		this->async(guestName, "login", "", timeout, MSTuple{username, password}, [](uint32_t userID)
		{
			MS_INFO("登录结果：%d", userID);
		});
	});
}

bool SpaceService::logout(uint32_t timeout)
{
	return this->async("guest", "guest", "", 100, MSTuple{}, [=, this](uint32_t guestID)
	{
		if (guestID == 0) return;
		auto guestName = "guest:" + std::to_string(guestID);

		MS_INFO("正在注销...");

		this->async(guestName, "logout", "", timeout, MSTuple{}, [](bool result)
		{
			MS_INFO("注销结果：%s", result ? "true" : "false");
		});
	});
}

MSAsync<void> SpaceService::onLogin(uint32_t userID)
{
	MS_INFO("登录回调：%u", userID);
	
	auto playerService = this->onCreatingPlayer(userID);
	if(this->create("client:" + std::to_string(userID), playerService))
	{
		m_UserID = userID;
		m_PlayerService = playerService;

		MS_INFO("用户 %u 登录成功", userID);
		co_await playerService->onCreatePlayer();
	}
	co_return;
}

MSAsync<void> SpaceService::onLogout(uint32_t userID, bool result)
{
	MS_INFO("注销回调：%s", result ? "true" : "false");

	if(this->cancel("client:" + std::to_string(userID)))
	{
		m_UserID = 0;
		m_PlayerService.reset();

		MS_INFO("用户 %u 注销成功", userID);
	}
	co_return;
}

MSAsync<void> SpaceService::onSignup(bool result)
{
	MS_INFO("注册回调：%s", result ? "true" : "false");
	co_return;
}

MSAsync<void> SpaceService::onEnterSpace(uint32_t spaceID, uint32_t userID)
{
	MS_INFO("用户 %u 进入空间 %u", userID, spaceID);
	co_return;
}

MSAsync<void> SpaceService::onLeaveSpace(uint32_t spaceID, uint32_t userID)
{
	MS_INFO("用户 %u 离开空间 %u", userID, spaceID);
	co_return;
}

MSRef<PlayerService> SpaceService::onCreatingPlayer(uint32_t userID)
{
    return MSNew<PlayerService>(userID);
}
