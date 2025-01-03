#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/MS.h>
#include "Player.h"

class PlayerManager
{
public:
	MSHnd<Player> getPlayer(uint32_t uid) const
	{
		auto result = m_Players.find(uid);
		if (result == m_Players.end()) return MSHnd<Player>();
		return result->second;
	}

	void setPlayer(uint32_t uid, MSRef<Player> player)
	{
		if (player) m_Players[uid] = player;
		else m_Players.erase(uid);
	}

protected:
	MSMap<uint32_t, MSRef<Player>> m_Players;
	MSMap<uint32_t, MSRef<Player>> m_BattlePlayers;
};