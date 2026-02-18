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
#include "ClientService.h"
#include "PlayerService.h"
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

	auto clientService = MSNew<ClientService>();
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
		MSString request(sizeof(MailView) + mail.Body.size(), '?');
		auto& mailView = *(MailView*)request.data();
		mailView.From = mail.From;
		mailView.To = mail.To;
		mailView.Copy = mail.Copy;
		mailView.Date = mail.Date;
		mailView.Type = mail.Type;
		if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
		m_TCPChannel->writeChannel(IChannelEvent::New(request));
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
							IMail mail = {};
							mail.From = mailView.From;
							mail.To = mailView.To;
							mail.Copy = mailView.Copy;
							mail.Date = mailView.Date;
							mail.Type = mailView.Type;
							mail.Body = MSStringView(mailView.Body, event->Message.size() - sizeof(MailView));
							MS_INFO("gate %u=>%u via %u #%u @%u", mail.From, mail.To, mail.Copy, mail.Date, mail.Type);
							mailHub->send(mail);
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

				clientService->async("guest", "guest", "", 5000, MSTuple{}, [=](uint32_t guestID)
				{
					if (guestID == 0) return;
					auto guestName = "guest:" + std::to_string(guestID);

					MS_INFO("尝试注册...");

					clientService->async(guestName, "signup", "", 5000, MSTuple{"openms", "123456"}, [=](bool result)
					{
						MS_INFO("注册结果：%d", result);

						MS_INFO("尝试登录...");
						clientService->async(guestName, "login", "", 5000, MSTuple{"openms", "123456"}, [=](uint32_t userID)
						{
							MS_INFO("登录结果：%u", userID);
							if (userID == 0) return;
							channel->getContext()->userdata() = userID;

							auto playerService = MSNew<PlayerService>(userID);
							mailHub->create("client:" + std::to_string(userID), playerService);

							MS_INFO("尝试匹配...");	auto gameID = 0;
							playerService->callServer("matchBattle", 5000, MSTuple{ gameID }, [=]() {});
						});
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
