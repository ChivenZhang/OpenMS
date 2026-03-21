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
#include <Service/StateSync/Frontend/FrontendClient.h>
#include <Service/StateSync/Frontend/SpaceService.h>

using namespace state_client;

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

    void onStateChange(MSStringView state, bool full) override
    {
        MS_DEBUG("玩家 %u 状态改变：%s 全量：%s", userID(), MSString(state.data(), state.size()).c_str(), full ? "yes" : "no");
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

            MS_INFO("input cmd: [login|logout|match|attack|clear|exit]");

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
                            playerService->callServer("matchBattle", 1000, MSTuple{ 0U }, [](bool result)
                            {
                                MS_INFO("匹配结果：%s", result ? "true" : "false");
                            });
                        }
                    }
                }
                else if(line == "attack")
                {
                    if(auto clientService = this->m_ClientService.lock())
                    {
                        if(auto playerService = clientService->player())
                        {
                            playerService->callPlayer("attack", 1000, MSTuple{ 0U }, []()
                            {
                            });
                        }
                    }
                }
                else if (line == "clear")
                {
#ifdef OPENMS_PLATFORM_WINDOWS
                    system("cls");
#else
                    system("clear");
#endif
                }
                else if(line == "exit")
                {
                    AUTOWIRE_DATA(IServer)->shutdown();
                    break;
                }

                MS_INFO("input cmd: [login|logout|match|attack|clear|exit]");
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