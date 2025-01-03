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
#include "Battle.h"

class BattleManager
{
public:
	MSHnd<Battle> getBattle(uint32_t uid) const
	{
		auto result = m_Battles.find(uid);
		if (result == m_Battles.end()) return MSHnd<Battle>();
		return result->second;
	}

	void setBattle(uint32_t uid, MSRef<Battle> battle)
	{
		if (battle) m_Battles[uid] = battle;
		else m_Battles.erase(uid);
	}

protected:
	MSMap<uint32_t, MSRef<Battle>> m_Battles;
};