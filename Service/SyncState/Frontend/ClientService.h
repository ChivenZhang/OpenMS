#pragma once
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
#include <Server/Private/Service.h>

class ClientService : public Service
{
public:
	ClientService();

	virtual bool signup(MSString username, MSString password, uint32_t timeout = 5000/*ms*/);

	virtual bool login(MSString username, MSString password, uint32_t timeout = 5000/*ms*/);

	virtual bool logout(uint32_t timeout = 5000/*ms*/);

	virtual MSAsync<void> onLogin(uint32_t userID);

	virtual MSAsync<void> onLogout(bool result);

	virtual MSAsync<void> onSignup(bool result);
};