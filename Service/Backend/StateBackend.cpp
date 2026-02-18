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
#include <Service/SyncState/Backend/PlayerService.h>

class MyPlayerService : public PlayerService
{
public:
    using PlayerService::PlayerService;
};

class MySpaceService : public SpaceService
{
public:
    using SpaceService::SpaceService;

protected:
    MSRef<Service> onCreatingPlayer(uint32_t userID) override
    {
        return MSNew<MyPlayerService>(userID);
    }
};

class MyBackendServer : public BackendServer
{
public:
    MSRef<Service> onCreatingSpace(uint32_t spaceID, uint32_t gameID) override
    {
        return MSNew<MySpaceService>(spaceID, gameID);
    }
};

OPENMS_RUN(MyBackendServer)