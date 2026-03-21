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
#include <OpenMS/Server/Private/Service.h>

class SpaceService : public Service
{
public:
	SpaceService();

	virtual MSAsync<void> onLogin(uint32_t userID);

	virtual MSAsync<void> onLogout(bool result);

	virtual MSAsync<void> onSignup(bool result);
};