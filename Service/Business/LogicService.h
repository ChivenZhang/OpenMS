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

	virtual MSAsync<uint32_t> createSpace();
	virtual MSAsync<bool> deleteSpace(uint32_t spaceID);
	virtual MSAsync<void> onSpaceCreate(uint32_t spaceID);
	virtual MSAsync<void> onSpaceDelete(uint32_t spaceID);

protected:
	MSMutex m_UserLock;
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