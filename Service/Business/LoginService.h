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

class LoginService : public Service
{
public:
	LoginService();

	virtual MSAsync<uint32_t> onLoginRequest(MSString caller, MSString username, MSString password);
	virtual MSAsync<void> onLoginFromDB(MSString caller, uint32_t userID, uint32_t code, MSString error);

	virtual MSAsync<uint32_t> onRequestSignup(MSString caller, MSString username, MSString password);
	virtual MSAsync<void> onSignupFromDB(MSString caller, uint32_t userID, uint32_t code, MSString error);
};