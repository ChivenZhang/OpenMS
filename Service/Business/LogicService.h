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

	virtual MSAsync<MSString> globalData(MSStringView name) final;
	virtual MSAsync<void> globalData(MSStringView name, MSStringView value) final;
	virtual MSAsync<void> onGlobalData(MSStringView name, MSStringView value) final;

	virtual MSAsync<uint32_t> onLoginRequest(MSString username, MSString password);
	virtual MSAsync<void> onClientLogin(uint32_t userID, uint32_t code, MSString error);

	virtual MSAsync<uint32_t> onSignupRequest(MSString username, MSString password);
	virtual MSAsync<void> onClientSignup(uint32_t userID, uint32_t code, MSString error);

	virtual MSAsync<void> onBattleMatch(uint32_t gameID, MSList<uint32_t> userIDs);

	virtual MSAsync<uint32_t> onSpaceRequest(uint32_t gameID);
	virtual MSAsync<void> onSpaceCreate(uint32_t spaceID);
	virtual MSAsync<void> onSpaceDelete(uint32_t spaceID);
	virtual MSAsync<void> onSpaceEnter(uint32_t spaceID, uint32_t userID);
	virtual MSAsync<void> onSpaceLeave(uint32_t spaceID, uint32_t userID);

protected:
	MSMutex m_UserLock;
	struct user_t
	{
		bool Online;
		bool InGame;
		uint32_t SpaceID;
		float LastUpdate;
	};
	uint32_t m_SpaceID = 0;
	MSMap<uint32_t, user_t> m_UserInfos;

	struct space_t
	{
		bool InGame;
		uint32_t GameID;
		MSList<uint32_t> UserIDs;
	};
	MSMap<uint32_t, space_t> m_SpaceInfos;
};