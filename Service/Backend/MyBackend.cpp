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
#include <Service/StateSync/Backend/BackendServer.h>
#include <Service/StateSync/Backend/SpaceService.h>

using namespace state_server;

class MyPlayer : public PlayerService
{
public:
    MyPlayer(uint32_t userID) : PlayerService(userID) 
    {
        this->bind("attack", [=, this]()->MSAsync<void>
        {
            MS_INFO("用户 {} 发动攻击", userID);
            co_await this->callClient<void>("onAttack", 0, MSTuple{});
        });
    }

protected:
    void onStateChange(MSString& data, bool full) override
    {
        // TODO: 处理状态改变
    }
};

class MySpace : public SpaceService
{
public:
    using SpaceService::SpaceService;

protected:
    MSRef<PlayerService> onCreatingPlayer(uint32_t userID) override
    {
        return MSNew<MyPlayer>(userID);
    }
};

class MyBackend : public BackendServer
{
public:
    MSRef<Service> onCreatingSpace(uint32_t spaceID, uint32_t gameID) override
    {
        return MSNew<MySpace>(spaceID, gameID);
    }
};

OPENMS_RUN(MyBackend)