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

	virtual MSAsync<void> onLogin(uint32_t userID);

	virtual MSAsync<void> onLogout(bool result);

	virtual MSAsync<void> onSignup(bool result);
};