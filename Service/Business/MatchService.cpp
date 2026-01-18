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
#include "MatchService.h"

MatchService::MatchService()
{
	this->bind("enterMatch", [=, this](uint32_t userID, uint32_t gameID)->MSAsync<bool>
	{
		if (m_UserMatches.emplace(userID, gameID).second)
		{
			auto& player = m_Candidates[gameID].emplace_back();
			player.UserID = userID;
			player.Level = 0;
			player.Score = 0;
			co_return true;
		}
		co_return false;
	});
	this->bind("leaveMatch", [=, this](uint32_t userID)->MSAsync<bool>
	{
		auto result = m_UserMatches.find(userID);
		if (result != m_UserMatches.end())
		{
			auto gameID = result->second;
			auto count = std::erase_if(m_Candidates[gameID], [=](auto& player)->bool{ return player.UserID == userID; });
			co_return count != 0;
		}
		co_return false;
	});
	this->bind("updateMatch", [=, this](MSString caller)->MSAsync<void>
	{
		for (auto& [gameID, players] : m_Candidates)
		{
			MSSet<uint32_t> result;
			co_await this->onMatchRequest(gameID, players, result);
			for (auto& userID : result) m_UserMatches.erase(userID);
			std::erase_if(players, [&](auto const& player) { return result.contains(player.UserID); });
			co_await this->onMatchComplete(caller, gameID, result);
		}
		co_return;
	});
}

MSAsync<void> MatchService::onMatchRequest(uint32_t gameID, MSList<player_t>& candidates, MSSet<uint32_t>& result)
{
	for (auto& player : candidates) result.insert(player.UserID);
	co_return;
}

MSAsync<void> MatchService::onMatchComplete(MSString caller, uint32_t gameID, MSSet<uint32_t>& participates)
{
	if (participates.empty()) co_return;
	co_return co_await this->async<void>(caller, "onBattleMatch", "", 0, MSTuple{gameID, participates});
}