/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "FrontendClient.h"
#include "Handler/AES/AESHandler.h"
#include "Mailbox/Private/Mail.h"
#include "Server/Private/Service.h"

#define GATEWAY_ADDR "127.0.0.1"
#define GATEWAY_PORT 9090
#define AES256_KEY 0x50, 0x71, 0x47, 0x55, 0x6f, 0x4c, 0x36, 0x7a, 0x55, 0x50, 0x38, 0x43, 0x30, 0x42, 0x38, 0x43, 0x2f, 0x55, 0x45, 0x4d, 0x50, 0x49, 0x73, 0x36, 0x4f, 0x63, 0x42, 0x4c, 0x79, 0x32, 0x72, 0x35

MSString FrontendClient::identity() const
{
	return "frontend";
}

void FrontendClient::onInit()
{
	auto mailHub = m_MailHub = MSNew<MailHub>();
	auto clientService = MSNew<Service>();
	clientService->bind("onLogin", [=](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("登录回调");
		co_return;
	});
	clientService->bind("onLogout", [=](bool result)->MSAsync<void>
	{
		MS_INFO("注销回调");
		co_return;
	});
	clientService->bind("onSignup", [=](bool result)->MSAsync<void>
	{
		MS_INFO("注册回调");
		co_return;
	});
	clientService->bind("onCreateClient", [=](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("创建角色");
		co_return;
	});
	clientService->bind("onCreateGhost", [=](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("创建代理");
		co_return;
	});
	clientService->bind("onDeleteGhost", [=](uint32_t userID)->MSAsync<void>
	{
		MS_INFO("删除角色/代理");
		co_return;
	});
	mailHub->create("client", clientService);

	// Handle local mail to remote

	mailHub->failed([=, this](IMail mail)->bool
	{
		auto client = m_TCPClient;
		if (client && client->connect() == false)
		{
			client->shutdown();
			client->startup();
		}
		if (client == nullptr || client->connect() == false) return false;
		MSString request(sizeof(MailView) + mail.Body.size(), 0);
		auto& mailView = *(MailView*)request.data();
		mailView.From = mail.From;
		mailView.To = mail.To;
		mailView.Copy = mail.Copy;
		mailView.Date = mail.Date;
		mailView.Type = mail.Type;
		if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
		m_TCPChannel->writeChannel(IChannelEvent::New(request));

		MS_INFO("客户请求：%u => %u", mailView.From, mailView.To);
		return true;
	});

	// Handle remote mail to local

	m_TCPClient = MSNew<TCPClient>(TCPClient::config_t{
		.IP = GATEWAY_ADDR,
		.PortNum = GATEWAY_PORT,
		.Workers = 1,
		.Callback = ChannelReactor::callback_t
		{
			.OnOpen = [=, this](MSRef<IChannel> channel)
			{
				m_TCPChannel = channel;

				channel->getPipeline()->addFirst("decrypt", MSNew<AESInboundHandler>(AESInboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("input", IChannelPipeline::handler_in
				{
					.OnHandle = [=](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						if (sizeof(MailView) <= event->Message.size())
						{
							auto& mailView = *(MailView*)event->Message.data();
							IMail newMail = {};
							newMail.From = mailView.From;
							newMail.To = mailView.To;
							newMail.Copy = mailView.Copy;
							newMail.Date = mailView.Date;
							newMail.Type = mailView.Type;
							newMail.Body = MSStringView(mailView.Body, event->Message.size() - sizeof(MailView));
							mailHub->send(newMail);
						}
						return false;
					},
				});
				channel->getPipeline()->addFirst("encrypt", MSNew<AESOutboundHandler>(AESOutboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("output", IChannelPipeline::handler_out
				{
					.OnHandle = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						context->write(IChannelEvent::New(event->Message));
						return false;
					},
				});

				MS_INFO("尝试登录...");
				clientService->async("guest", "login", "", 5000, R"(["admin","123456"])", [=](MSStringView&& response)
				{
					MS_INFO("登录结果：%s", response.data());
					if (response.empty()) return;
					auto userID = std::stoul(response.data());
					if (userID == 0) return;
					channel->getContext()->userdata() = userID;

					auto playerService = MSNew<Service>();
					playerService->bind("onStartBattle", [=, _this = playerService.get()]()->MSAsync<void>
					{
						MS_INFO("开始游戏");

						MS_INFO("尝试攻击...");
						_this->async("player:" + std::to_string(userID), "attack", "proxy:" + std::to_string(userID), 100, MSTuple{}, [=](bool result)
						{
							MS_INFO("攻击结果：%d", result);

							MS_INFO("尝试登出...");
							_this->async("guest", "logout", "", 1000, MSTuple{userID}, [=](bool result2)
							{
								MS_INFO("登出结果：%d", result2);
							});
						});
						co_return;
					});
					playerService->bind("onAttack", [=]()->MSAsync<void>
					{
						MS_INFO("发动攻击");
						co_return;
					});
					playerService->bind("onStopBattle", [=]()->MSAsync<void>
					{
						MS_INFO("结束游戏");
						co_return;
					});
					mailHub->create("client:" + std::to_string(userID), playerService);

					MS_INFO("尝试开局...");
					playerService->async("server:" + std::to_string(userID), "readyBattle", "proxy:" + std::to_string(userID), 30000, MSTuple{ 0U }, [=](bool result)
					{
						MS_INFO("开局结果：%d", result);
					});
				});
			},
		}
	});
	m_TCPClient->startup();
}

void FrontendClient::onExit()
{
	if (m_TCPClient) m_TCPClient->shutdown();
	m_TCPClient = nullptr;

	m_MailHub = nullptr;
}

void FrontendClient::onUpdate(float time)
{
}

OPENMS_RUN(FrontendClient);