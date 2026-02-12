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

class MatchService : public Service
{
protected:
	using user_t = uint32_t;
	using game_t = uint32_t;
	struct player_t
	{
		user_t UserID;
		uint32_t Level;
		float Score;
	};

public:
	MatchService();

	virtual MSAsync<void> onMatchRequest(uint32_t gameID, MSList<player_t>& candidates, MSSet<uint32_t>& result);

	virtual MSAsync<void> onMatchComplete(MSString caller, uint32_t gameID, MSSet<uint32_t>& userIDs);

protected:
	MSMap<user_t, game_t> m_UserMatches;
	MSMap<game_t, MSList<player_t>> m_Candidates;
};