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

SpaceService::SpaceService()
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

MSAsync<void> SpaceService::onLogin(uint32_t userID)
{
	co_return;
}

MSAsync<void> SpaceService::onLogout(bool result)
{
	co_return;
}

MSAsync<void> SpaceService::onSignup(bool result)
{
	co_return;
}
