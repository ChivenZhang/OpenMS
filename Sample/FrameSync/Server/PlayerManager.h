#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/MS.h>
#include "Player.h"

class PlayerManager
{
public:
	THnd<Player> getPlayer(uint32_t uid) const
	{
		auto result = m_Players.find(uid);
		if (result == m_Players.end()) return THnd<Player>();
		return result->second;
	}

	void setPlayer(uint32_t uid, TRef<Player> player)
	{
		if (player) m_Players[uid] = player;
		else m_Players.erase(uid);
	}

protected:
	TMap<uint32_t, TRef<Player>> m_Players;
	TMap<uint32_t, TRef<Player>> m_BattlePlayers;
};