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
#include "ClientService.h"

ClientService::ClientService()
{
	this->bind("onLogin", [=, this](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("登录回调");
		co_return co_await this->onLogin(userID);
	});
	this->bind("onLogout", [=, this](bool result)->MSAsync<void>
	{
		MS_INFO("注销回调");
		co_return co_await this->onLogout(result);
	});
	this->bind("onSignup", [=, this](bool result)->MSAsync<void>
	{
		MS_INFO("注册回调");
		co_return co_await this->onSignup(result);
	});
	this->bind("onCreateClient", [=](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("创建角色");
		co_return;
	});
	this->bind("onCreateGhost", [=](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("创建代理");
		co_return;
	});
	this->bind("onDeleteGhost", [=](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("删除角色/代理");
		co_return;
	});
}

bool ClientService::signup(MSString username, MSString password, uint32_t timeout)
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

bool ClientService::login(MSString username, MSString password, uint32_t timeout)
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

bool ClientService::logout(uint32_t timeout)
{
	return this->async("guest", "guest", "", 100, MSTuple{}, [=, this](uint32_t guestID)
	{
		if (guestID == 0) return;
		auto guestName = "guest:" + std::to_string(guestID);

		MS_INFO("正在注销...");

		this->async(guestName, "logout", "", timeout, MSTuple{}, [](bool result)
		{
			MS_INFO("注销结果：%d", result ? "true" : "false");
		});
	});
}

MSAsync<void> ClientService::onLogin(uint32_t userID)
{
	MS_INFO("登录回调：%u", userID);
	co_return;
}

MSAsync<void> ClientService::onLogout(bool result)
{
	MS_INFO("注销回调：%s", result ? "true" : "false");
	co_return;
}

MSAsync<void> ClientService::onSignup(bool result)
{
	MS_INFO("注册回调：%s", result ? "true" : "false");
	co_return;
}
