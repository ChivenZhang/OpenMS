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
#include "SpaceService.h"

namespace state_client
{
	SpaceService::SpaceService()
	{
		this->bind("onLogin", [=, this](uint32_t userID)->MSAsync<void>
		{
			co_return co_await this->onLogin(userID);
		});
		this->bind("onLogout", [=, this](uint32_t userID, bool result)->MSAsync<void>
		{
			co_return co_await this->onLogout(userID, result);
		});
		this->bind("onSignup", [=, this](bool result)->MSAsync<void>
		{
			co_return co_await this->onSignup(result);
		});
		this->bind("onEnterSpace", [=, this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
		{
			if(this->exist("client:" + std::to_string(userID)) == false)
			{
				auto playerService = this->onCreatingPlayer(userID);
				if (this->create("client:" + std::to_string(userID), playerService))
				{
					auto& user = m_UserInfos[userID];
					user.Player = playerService;
					co_await playerService->onCreatePlayer();
				}
			}
			co_return co_await this->onEnterSpace(spaceID, userID);
		});
		this->bind("onLeaveSpace", [=, this](uint32_t spaceID, uint32_t userID)->MSAsync<void>
		{
			if(this->exist("client:" + std::to_string(userID)) == true)
			{
				auto playerService = m_UserInfos.emplace(userID, user_t{}).first->second.Player.lock();
				if (this->cancel("client:" + std::to_string(userID)))
				{
					m_UserInfos.erase(userID);
					co_await playerService->onDeletePlayer();
				}
			}
			co_return co_await this->onLeaveSpace(spaceID, userID);
		});
		this->bind("onStateChange", [=, this](uint32_t userID, MSString const& state, bool full)->MSAsync<void>
		{
			// Route states to any player service

			if (auto result = m_UserInfos.find(userID); result != m_UserInfos.end())
			{
				if (auto& user = result->second; auto player = user.Player.lock())
				{
					player->onStateChange(state, full);
				}
			}
			co_return;
		});
	}

	uint32_t SpaceService::userID() const
	{
		return m_UserID;
	}

	MSRef<PlayerService> SpaceService::player() const
	{
		return m_PlayerService.lock();
	}

	bool SpaceService::signup(MSString username, MSString password, uint32_t timeout)
	{
		return this->async("guest", "guest", "", 100, MSTuple{}, [=, this](uint32_t guestID)
		{
			if (guestID == 0) return;
			auto guestName = "guest:" + std::to_string(guestID);

			MS_INFO("正在注册...");

			this->async(guestName, "signup", "", timeout, MSTuple{username, password}, [](bool result)
			{
				MS_INFO("注册结果：%s", result ? "true" : "false");
			});
		});
	}

	bool SpaceService::login(MSString username, MSString password, uint32_t timeout)
	{
		return this->async("guest", "guest", "", 100, MSTuple{}, [=, this](uint32_t guestID)
		{
			if (guestID == 0) return;
			auto guestName = "guest:" + std::to_string(guestID);

			MS_INFO("正在登录...");

			this->async(guestName, "login", "", timeout, MSTuple{username, password}, [](uint32_t userID)
			{
				MS_INFO("登录结果：%d", userID);
			});
		});
	}

	bool SpaceService::logout(uint32_t timeout)
	{
		return this->async("guest", "guest", "", 100, MSTuple{}, [=, this](uint32_t guestID)
		{
			if (guestID == 0) return;
			auto guestName = "guest:" + std::to_string(guestID);

			MS_INFO("正在注销...");

			this->async(guestName, "logout", "", timeout, MSTuple{}, [](bool result)
			{
				MS_INFO("注销结果：%s", result ? "true" : "false");
			});
		});
	}

	MSAsync<void> SpaceService::onLogin(uint32_t userID)
	{
		MS_INFO("登录回调：%u", userID);

		auto playerService = this->onCreatingPlayer(userID);
		if (this->create("client:" + std::to_string(userID), playerService))
		{
			m_UserID = userID;
			m_PlayerService = playerService;

			auto& user = m_UserInfos[userID];
			user.Player = playerService;
			co_await playerService->onCreatePlayer();
		}
		co_return;
	}

	MSAsync<void> SpaceService::onLogout(uint32_t userID, bool result)
	{
		MS_INFO("注销回调：%s", result ? "true" : "false");

		if(this->cancel("client:" + std::to_string(userID)))
		{
			m_UserID = 0;
			m_PlayerService.reset();

			MS_INFO("用户 %u 注销成功", userID);
		}
		co_return;
	}

	MSAsync<void> SpaceService::onSignup(bool result)
	{
		MS_INFO("注册回调：%s", result ? "true" : "false");
		co_return;
	}

	MSAsync<void> SpaceService::onEnterSpace(uint32_t spaceID, uint32_t userID)
	{
		MS_INFO("用户 %u 进入空间 %u", userID, spaceID);
		co_return;
	}

	MSAsync<void> SpaceService::onLeaveSpace(uint32_t spaceID, uint32_t userID)
	{
		MS_INFO("用户 %u 离开空间 %u", userID, spaceID);
		co_return;
	}

	MSRef<PlayerService> SpaceService::onCreatingPlayer(uint32_t userID)
	{
		return MSNew<PlayerService>(userID);
	}
}