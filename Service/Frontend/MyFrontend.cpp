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
#include <Service/SyncState/Frontend/FrontendClient.h>
#include <Service/SyncState/Frontend/ClientService.h>

class MyPlayer : public PlayerService
{
public:
    explicit MyPlayer(uint32_t userID) : PlayerService(userID)
    {
        this->bind("onAttack", [=]()->MSAsync<void>
        {
            MS_INFO("发动攻击");
            co_return;
        });
    }

protected:
    MSAsync<void> onCreatePlayer() override
    {
        co_await this->callServer<void>("matchBattle", 5000, MSTuple{ 0U });

        co_return;
    }
    MSAsync<void> onStartBattle() override
    {
        co_await PlayerService::onStartBattle();

        co_await this->callPlayer<void>("attack", 0, MSTuple{});
    }

    MSAsync<void> onStopBattle() override
    {
        co_await PlayerService::onStopBattle();

	    AUTOWIRE_DATA(IServer)->shutdown();
    }

    MSAsync<void> onStateChange(MSStringView state) override
    {
        MS_INFO("玩家 %u 状态改变：%s", m_UserID, state.data());
        // TODO:
        co_return;
    }
};

class MyClient : public ClientService
{
protected:
    MSRef<PlayerService> onCreatingPlayer(uint32_t userID) override
    {
        return MSNew<MyPlayer>(userID);
    }
};

class MyFrontend : public FrontendClient
{
protected:
    void onInit() override
    {
        FrontendClient::onInit();

        if(auto clientService = m_ClientService.lock())
        {
            clientService->login("admin", "123456");
        }

        // TODO: Reconnect when disconnect.
    }
    MSRef<ClientService> onCreatingClient() override
    {
        return MSNew<MyClient>();
    }
};

OPENMS_RUN(MyFrontend)