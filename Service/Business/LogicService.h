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

class LogicService : public Service
{
public:
	LogicService();

	virtual MSAsync<uint32_t> onLoginRequest(MSStringView username, MSStringView password);

	virtual MSAsync<uint32_t> onCreateSpace();

	virtual MSAsync<bool> onDeleteSpace(uint32_t spaceID);

protected:
	MSMutex m_UserLock;
	MSMutex m_BattleLock;
	struct userinfo_t
	{
		bool Online;
		uint32_t SpaceID;
		float LastUpdate;
	};
	uint32_t m_SpaceID = 0;
	MSQueue<uint32_t> m_MatchQueue;
	MSMap<uint32_t, userinfo_t> m_UserInfos;
};