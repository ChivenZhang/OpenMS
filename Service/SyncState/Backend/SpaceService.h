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

protected:
	virtual MSRef<PlayerService> onCreatingPlayer(uint32_t userID);

protected:
	const uint32_t m_SpaceID;
	const uint32_t m_GameID;
	enum state_t
	{
		NONE = 0, MATCH, START, STOP,
	} m_GameState;
	struct user_t
	{
		uint32_t UserID;
	};
	MSMap<uint32_t, user_t> m_UserInfos;
	TimerUtility m_Timer;
};