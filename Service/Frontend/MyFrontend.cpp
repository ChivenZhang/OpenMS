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
#include <Service/SyncState/Frontend/SpaceService.h>

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
        co_return;
    }

    MSAsync<void> onStartBattle() override
    {
        co_await PlayerService::onStartBattle();
    }

    MSAsync<void> onStopBattle() override
    {
        co_await PlayerService::onStopBattle();
    }

    MSAsync<void> onStateChange(MSStringView state) override
    {
        MS_INFO("玩家 %u 状态改变：%s", this->userID(), state.data());
        // TODO:
        co_return;
    }
};

class MySpace : public SpaceService
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

        MSThread thread([=, this](){

            MS_INFO("input cmd: [login|logout|match|attack|exit]");

            std::string line;
            while(std::getline(std::cin, line))
            {
                if(line == "login")
                {
                    if(auto clientService = this->m_ClientService.lock())
                    {
                        clientService->login("admin", "123456");
                    }
                }
                else if(line == "logout")
                {
                    if(auto clientService = this->m_ClientService.lock())
                    {
                        clientService->logout();
                    }
                }
                else if(line == "match")
                {
                    if(auto clientService = this->m_ClientService.lock())
                    {
                        if(auto playerService = clientService->player())
                        {
                            playerService->callServer<void>("matchBattle", 0, MSTuple{ 0U });
                        }
                    }
                }
                else if(line == "attack")
                {
                    if(auto clientService = this->m_ClientService.lock())
                    {
                        if(auto playerService = clientService->player())
                        {
                            playerService->callPlayer<void>("attack", 0, MSTuple{ 0U });
                        }
                    }
                }
                else if(line == "exit")
                {
                    AUTOWIRE_DATA(IServer)->shutdown();
                    break;
                }
            }
        });
        thread.detach();

        // TODO: Reconnect when disconnect.
    }
    MSRef<SpaceService> onCreatingSpace() override
    {
        return MSNew<MySpace>();
    }
};

OPENMS_RUN(MyFrontend)