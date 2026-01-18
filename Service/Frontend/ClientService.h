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

	template<class F, class...Args>
	bool callGuest(MSStringView method, uint32_t timeout, MSTuple<Args...>&& args, F&& callback)
	{
		return this->async("guest", method, "", timeout, std::forward<MSTuple<Args...>>(args), std::forward<F>(callback));
	}

	template<class T, class... Args>
	MSAsync<T> callGuest(MSStringView method, uint32_t timeout, MSTuple<Args...>&& args)
	{
		co_return co_await this->async<T>("guest", method, "", timeout, std::forward<MSTuple<Args...>>(args));
	}

	virtual MSAsync<void> onLogin(uint32_t userID);

	virtual MSAsync<void> onLogout(bool result);

	virtual MSAsync<void> onSignup(bool result);
};