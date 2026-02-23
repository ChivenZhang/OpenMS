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
#include "PlayerService.h"

class SpaceService : public Service
{
public:
	explicit SpaceService(uint32_t spaceID, uint32_t gameID);

	uint32_t spaceID() const;

	uint32_t gameID() const;

	template<class F, class...Args>
	bool callClient(uint32_t userID, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args, F&& callback)
	{
		return this->async("client:" + std::to_string(userID), method, "proxy:" + std::to_string(userID), timeout, std::forward<MSTuple<Args...>>(args), std::forward<F>(callback));
	}

	template<class F, class...Args>
	bool callServer(uint32_t userID, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args, F&& callback)
	{
		return this->async("server:" + std::to_string(userID), method, "", timeout, std::forward<MSTuple<Args...>>(args), std::forward<F>(callback));
	}

	template<class F, class...Args>
	bool callPlayer(uint32_t userID, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args, F&& callback)
	{
		return this->async("player:" + std::to_string(userID), method, "", timeout, std::forward<MSTuple<Args...>>(args), std::forward<F>(callback));
	}

	template<class T, class... Args>
	MSAsync<T> callClient(uint32_t userID, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args)
	{
		co_return co_await this->async<T>("client:" + std::to_string(userID), method, "proxy:" + std::to_string(userID), timeout, std::forward<MSTuple<Args...>>(args));
	}

	template<class T, class... Args>
	MSAsync<T> callServer(uint32_t userID, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args)
	{
		co_return co_await this->async<T>("server:" + std::to_string(userID), method, "", timeout, std::forward<MSTuple<Args...>>(args));
	}

	template<class T, class... Args>
	MSAsync<T> callPlayer(uint32_t userID, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args)
	{
		co_return co_await this->async<T>("player:" + std::to_string(userID), method, "", timeout, std::forward<MSTuple<Args...>>(args));
	}

protected:
	virtual MSRef<PlayerService> onCreatingPlayer(uint32_t userID);

protected:
	const uint32_t m_SpaceID, m_GameID;
	enum state_t
	{
		NONE = 0, START, STOP,
	} m_GameState;
	struct user_t
	{
		MSHnd<PlayerService> Player;
	};
	MSMap<uint32_t, user_t> m_UserInfos;
	TimerUtility m_Timer;
};