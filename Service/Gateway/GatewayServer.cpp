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
#include "GatewayServer.h"
#include "GuestService.h"
#include "ProxyService.h"
#include "Endpoint/TCP/TCPServer.h"
#include "Handler/AES/AESHandler.h"
#include "Mailbox/Private/Mail.h"
#include "Server/Private/Service.h"

#define AES256_KEY 0x50, 0x71, 0x47, 0x55, 0x6f, 0x4c, 0x36, 0x7a, 0x55, 0x50, 0x38, 0x43, 0x30, 0x42, 0x38, 0x43, 0x2f, 0x55, 0x45, 0x4d, 0x50, 0x49, 0x73, 0x36, 0x4f, 0x63, 0x42, 0x4c, 0x79, 0x32, 0x72, 0x35

MSString GatewayServer::identity() const
{
	return "gateway";
}

void GatewayServer::onInit()
{
	ClusterServer::onInit();

	auto mailHub = AUTOWIRE(IMailHub)::bean();

	m_TCPServer = MSNew<TCPServer>(TCPServer::config_t{
		.IP = property(identity() + ".client.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".client.port", 0U),
		.Backlog = property(identity() + ".client.backlog", 0U),
		.Workers = property(identity() + ".client.workers", 0U),
		.Callback = ChannelReactor::callback_t
		{
			.OnOpen = [=, this](MSRef<IChannel> channel)
			{
				channel->getContext()->userdata() = 0;
				auto guestID = m_ClientCount.fetch_add(1) + 1;
				channel->getContext()->attribs()["guestID"] = guestID;

				// Create Guest Service

				constexpr auto H = MSHash("login");
				auto guestService = MSNew<GuestService>(channel, guestID);
				guestService->bind("login", [=, this](MSString user, MSString pass)-> MSAsync<uint32_t>
				{
					if (auto userID = channel->getContext()->userdata()) co_return userID;

					co_return co_await [=, this](MSAwait<uint32_t> promise)
					{
						MS_INFO("服务端验证：%s", user.c_str());

						guestService->async("logic", "login", "", 10000, MSTuple{user, pass}, [=, this](uint32_t userID)
						{
							if (userID)
							{
								auto serviceName = "proxy:" + std::to_string(userID);
								auto proxyService = MSNew<ProxyService>(channel, userID);
								if (mailHub->create(serviceName, proxyService)) this->onPush();
								channel->getContext()->userdata() = userID;
								MS_INFO("验证成功！ %s", user.c_str());
							}
							else
							{
								MS_INFO("验证失败！ %s", user.c_str());
							}
							promise(userID);
						});
					};
				});
				guestService->bind("logout", [=]()-> MSAsync<bool>
				{
					auto userID = channel->getContext()->userdata();
					if (userID == 0) co_return false;
					co_return co_await [=](MSAwait<bool> promise)
					{
						guestService->async("logic", "logout", "", 500, MSTuple{userID}, [=](bool result)
						{
							promise(result);
						});
					};
				});
				guestService->bind("signup", [=](MSString user, MSString pass)-> MSAsync<bool>
				{
					co_return co_await [=](MSAwait<bool> promise)
					{
						guestService->async("logic", "signup", "", 500, MSTuple{user, pass}, [=](bool result)
						{
							promise(result);
						});
					};
				});

				if (mailHub->create("guest:" + std::to_string(guestID), guestService)) this->onPush();

				// Create Client Channel

				channel->getPipeline()->addFirst("decrypt", MSNew<AESInboundHandler>(AESInboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("input", IChannelPipeline::handler_in
				{
					.OnHandle = [=](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						// TODO: 检查接口访问权限
						if (event->Message.size() < sizeof(MailView)) return false;
						auto& mailView = *(MailView*)event->Message.data();
						if (mailView.To == MSHash("guest"))
						{
							IMail newMail = {};
							newMail.From = mailView.From;
							newMail.To = MSHash("guest:" + std::to_string(guestID));
							newMail.Copy = MSHash(nullptr);
							newMail.Date = mailView.Date;
							newMail.Type = mailView.Type | OPENMS_MAIL_TYPE_CLIENT;
							newMail.Body = MSStringView(mailView.Body, event->Message.size() - sizeof(MailView));
							mailHub->send(newMail);
						}
						else
						{
							auto userID = (uint32_t)channel->getContext()->userdata();
							if (userID == 0) return false;
							IMail newMail = {};
							newMail.From = mailView.From;
							newMail.To = mailView.To;
							newMail.Copy = MSHash("proxy:" + std::to_string(userID));
							newMail.Date = mailView.Date;
							newMail.Type = mailView.Type | OPENMS_MAIL_TYPE_FORWARD;
							newMail.Body = MSStringView(mailView.Body, event->Message.size() - sizeof(MailView));
							if (newMail.Type & OPENMS_MAIL_TYPE_REQUEST) newMail.Type |= OPENMS_MAIL_TYPE_CLIENT;
							if (newMail.Type & OPENMS_MAIL_TYPE_RESPONSE) newMail.Type &= ~OPENMS_MAIL_TYPE_CLIENT;
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
			},
			.OnClose = [=, this](MSRef<IChannel> channel)
			{
				auto& guestData = channel->getContext()->attribs()["guestID"];
				auto guestID = std::any_cast<uint32_t>(guestData);
				auto serviceName = "guest:" + std::to_string(guestID);
				auto result = mailHub->cancel(serviceName);

				if (auto userID = (uint32_t)channel->getContext()->userdata())
				{
					serviceName = "proxy:" + std::to_string(userID);
					result |= mailHub->cancel(serviceName);
					channel->getContext()->userdata() = 0;
				}
				if (result) this->onPush();
			},
		}
	});
	m_TCPServer->startup();

	m_KeepAlive = startTimer(0, OPENMS_HEARTBEAT * 1000, [this](uint32_t handle)
	{
		if (m_TCPServer->connect() == false)
		{
			m_TCPServer->shutdown();
			m_TCPServer->startup();
		}
	});
}

void GatewayServer::onExit()
{
	if (m_KeepAlive) stopTimer(m_KeepAlive);
	m_KeepAlive = 0;

	if (m_TCPServer) m_TCPServer->shutdown();
	m_TCPServer = nullptr;

	ClusterServer::onExit();
}