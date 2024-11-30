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
#include "Battle.h"

class BattleManager
{
public:
	THnd<Battle> getBattle(uint32_t uid) const
	{
		auto result = m_Battles.find(uid);
		if (result == m_Battles.end()) return THnd<Battle>();
		return result->second;
	}

	void setBattle(uint32_t uid, TRef<Battle> battle)
	{
		if (battle) m_Battles[uid] = battle;
		else m_Battles.erase(uid);
	}

protected:
	TMap<uint32_t, TRef<Battle>> m_Battles;
};