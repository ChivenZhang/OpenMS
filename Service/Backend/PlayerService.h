#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include <Server/Private/Service.h>

class PlayerService : public Service
{
public:
	explicit PlayerService(uint32_t userID);

	template<class F, class...Args>
	bool callClient(MSStringView method, uint32_t timeout, MSTuple<Args...>&& args, F&& callback)
	{
		return this->async("client:" + std::to_string(m_UserID), method, "proxy:" + std::to_string(m_UserID), timeout, std::forward<MSTuple<Args...>>(args), std::forward<F>(callback));
	}

	template<class F, class...Args>
	bool callServer(MSStringView method, uint32_t timeout, MSTuple<Args...>&& args, F&& callback)
	{
		return this->async("server:" + std::to_string(m_UserID), method, "proxy:" + std::to_string(m_UserID), timeout, std::forward<MSTuple<Args...>>(args), std::forward<F>(callback));
	}

	template<class T, class... Args>
	MSAsync<T> callClient(MSStringView method, uint32_t timeout, MSTuple<Args...>&& args)
	{
		co_return co_await this->async<T>("client:" + std::to_string(m_UserID), method, "proxy:" + std::to_string(m_UserID), timeout, std::forward<MSTuple<Args...>>(args));
	}

	template<class T, class... Args>
	MSAsync<T> callServer(MSStringView method, uint32_t timeout, MSTuple<Args...>&& args)
	{
		co_return co_await this->async<T>("server:" + std::to_string(m_UserID), method, "", timeout, std::forward<MSTuple<Args...>>(args));
	}

protected:
	const uint32_t m_UserID;
};