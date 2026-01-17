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
#include "LoginService.h"

LoginService::LoginService()
{
	this->bind("login", [this](MSString caller, MSString user, MSString pass)->MSAsync<uint32_t>
	{
		MS_INFO("登录请求DB");
		auto userID = co_await this->onLoginRequest(caller, user, pass);
		co_return userID;
	});
	this->bind("signup", [this](MSString caller, MSString user, MSString pass)->MSAsync<bool>
	{
		MS_INFO("注册请求DB");
		auto userID = co_await this->onSignupRequest(caller, user, pass);
		co_await this->async<void>(this->name(), "onSignupFromDB", "", 0, MSTuple{caller, userID, 0, "success"});
		co_return userID != 0;
	});
}

MSAsync<uint32_t> LoginService::onLoginRequest(MSString caller, MSString username, MSString password)
{
	auto userID = co_await this->async<uint32_t>("userdb", "loginDB", "", 1000, MSTuple{username, password});
	co_await this->onLoginFromDB(caller, userID, 0, {});
	co_return userID;
}

MSAsync<void> LoginService::onLoginFromDB(MSString caller, uint32_t userID, uint32_t code, MSString error)
{
	co_return co_await this->async<void>(caller, "onClientLogin", "", 0, MSTuple{userID, code, error});
}

MSAsync<uint32_t> LoginService::onSignupRequest(MSString caller, MSString username, MSString password)
{
	// TODO: DB query
	static uint32_t s_UserID = 0;

	auto userID = ++s_UserID;
	co_await this->async<void>(this->name(), "onSignupFromDB", "", 0, MSTuple{caller, userID, 0, "success"});
	co_return userID;
}

MSAsync<void> LoginService::onSignupFromDB(MSString caller, uint32_t userID, uint32_t code, MSString error)
{
	co_return co_await this->async<void>(caller, "onClientSignup", "", 0, MSTuple{userID, code, error});
}