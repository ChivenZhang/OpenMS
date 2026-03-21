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
#include "PlayerService.h"

namespace state_client
{
	class SpaceService : public Service
	{
	public:
		SpaceService();

		uint32_t userID() const;

		MSRef<PlayerService> player() const;

		virtual bool signup(MSString username, MSString password, uint32_t timeout = 5000/*ms*/);

		virtual bool login(MSString username, MSString password, uint32_t timeout = 5000/*ms*/);

		virtual bool logout(uint32_t timeout = 5000/*ms*/);

	protected:
		virtual MSAsync<void> onLogin(uint32_t userID);

		virtual MSAsync<void> onLogout(uint32_t userID, bool result);

		virtual MSAsync<void> onSignup(bool result);

		virtual MSAsync<void> onEnterSpace(uint32_t spaceID, uint32_t userID);

		virtual MSAsync<void> onLeaveSpace(uint32_t spaceID, uint32_t userID);

		virtual MSRef<PlayerService> onCreatingPlayer(uint32_t userID);

	protected:
		uint32_t m_UserID;
		MSHnd<PlayerService> m_PlayerService;
		struct user_t
		{
			MSHnd<PlayerService> Player;
		};
		MSMap<uint32_t, user_t> m_UserInfos;
	};
}