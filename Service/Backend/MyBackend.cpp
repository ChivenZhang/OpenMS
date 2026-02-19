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
#include <Service/SyncState/Backend/BackendServer.h>
#include <Service/SyncState/Backend/SpaceService.h>

class MyPlayer : public PlayerService
{
public:
    MyPlayer(uint32_t userID) : PlayerService(userID) 
    {
        this->bind("attack", [=, this]()->MSAsync<void>
        {
            MS_INFO("用户 %u 发动攻击", userID);
            co_await this->callClient<void>("onAttack", 0, MSTuple{});
        });
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