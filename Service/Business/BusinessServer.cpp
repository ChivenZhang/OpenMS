/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "BusinessServer.h"
#include "PlayerService.h"
#include "ServerService.h"
#include "Mailbox/Private/Mail.h"
#include "Server/Private/Service.h"

MSString BusinessServer::identity() const
{
	return "business";
}

void BusinessServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	auto logicService = MSNew<Service>();

	// Login/out Module

	logicService->bind("login", [=, this, _logic = logicService.get()](MSString user, MSString pass)->MSAsync<uint32_t>
	{
		MS_INFO("服务端：LOGIN!!!");
		static uint32_t s_UserID = 0;
		auto userID = ++s_UserID;

		{
			MSMutexLock lock(m_UserLock);
			auto& userInfo = m_UserInfos[userID];
			userInfo.Online = true;
			userInfo.LastUpdate = 1.0f * ::clock() / CLOCKS_PER_SEC;
		}

		auto serverService = MSNew<ServerService>();
		serverService->bind("readyBattle", [=, this](uint32_t gameID)->MSAsync<bool>
		{
			MS_INFO("服务端：READY BATTLE!!!");
			m_MatchQueue.push(userID);

			auto playerService = MSNew<PlayerService>(userID);
			mailHub->create("player:" + std::to_string(userID), playerService);

			// Match battle
			if (2 <= m_MatchQueue.size())
			{
				for (auto i=0; i<2; ++i)
				{
					auto userID2 = m_MatchQueue.front();
					m_MatchQueue.pop();
					_logic->call<bool>("player:" + std::to_string(userID2), "startBattle", "", 0, MSTuple{});
				}
			}

			co_return true;
		});
		serverService->bind("notifyBattle", [=, this]()->MSAsync<void>
		{
			MSMutexLock lock(m_BattleLock);
			auto result = m_UserInfos.find(userID);
			if (result != m_UserInfos.end() && result->second.Online)
			{
				auto& userInfo = result->second;
				userInfo.LastUpdate = 1.0f * ::clock() / CLOCKS_PER_SEC;
			}
			co_return;
		});
		mailHub->create("server:" + std::to_string(userID), serverService);
		co_return userID;
	});
	logicService->bind("logout", [=, this](uint32_t userID)->MSAsync<bool>
	{
		MS_INFO("服务端：LOGOUT!!!");	// TODO: 心跳超时，自动登出
		auto result = false;
		result |= mailHub->cancel("server:" + std::to_string(userID));
		result |= mailHub->cancel("player:" + std::to_string(userID));
		if (result)
		{
			MSMutexLock lock(m_UserLock);
			auto& userInfo = m_UserInfos[userID];
			userInfo.Online = false;
		}
		co_return true;
	});
	logicService->bind("signup", [](MSString user, MSString pass)->MSAsync<bool>
	{
		MS_INFO("服务端：SIGNUP!!!");
		co_return false;
	});

	// Match Module

	logicService->bind("matchBattle", [_this = logicService.get()](uint32_t userID)->MSAsync<uint32_t>
	{
		MS_INFO("服务端：START BATTLE!!!");	// Assume battle ready
		_this->call<bool>("player:" + std::to_string(userID), "startBattle", "", 0, MSTuple{});
		co_return 0;
	});

	mailHub->create("logic", logicService);

	startTimer(0, 2000, [=, this](uint32_t handle)
	{
		MSMutexLock lock(m_UserLock);
		auto now = 1.0f * ::clock() / CLOCKS_PER_SEC;
		for (auto& userInfo : m_UserInfos)
		{
			if (userInfo.second.Online && userInfo.second.LastUpdate + 10.0f <= now)
			{
				logicService->call<bool>("logic", "logout", "", 0, MSTuple{ userInfo.first });
			}
		}
	});
}

void BusinessServer::onExit()
{
	ClusterServer::onExit();
}