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
				const auto guestID = m_GuestID.fetch_add(1) + 1;
				channel->getContext()->attribs()["guestID"] = guestID;

				// Create Guest Service

				auto guestService = MSNew<GuestService>(channel, guestID);
				mailHub->create("guest:" + std::to_string(guestID), guestService);

				// Create Client Channel

				channel->getPipeline()->addFirst("decrypt", MSNew<AESInboundHandler>(AESInboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("input", IChannelPipeline::handler_in
				{
					.OnHandle = [=, guestName = MSHash("guest:" + std::to_string(guestID))](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						if (event->Message.size() < sizeof(MailView)) return false;
						auto& mailView = *(MailView*)event->Message.data();
						if (mailView.To == MSHash("guest"))
						{
							auto body = std::to_string(guestID);
							MSString buffer(sizeof(MailView) + body.size(), '?');
							auto& mail = *(MailView*)buffer.data();
							mail.From = mailView.From;
							mail.To = mailView.To;
							mail.Copy = mailView.Copy;
							mail.Date = mailView.Date;
							mail.Type = mailView.Type;
							if (body.empty() == false) ::memcpy(mail.Body, body.data(), body.size());
							std::swap(mail.From, mail.To);
							mail.Type &= ~OPENMS_MAIL_TYPE_REQUEST;
							mail.Type |= OPENMS_MAIL_TYPE_RESPONSE;
							channel->writeChannel(IChannelEvent::New(buffer));
						}
						else if (mailView.To == guestName)
						{
							IMail mail = {};
							mail.From = mailView.From;
							mail.To = mailView.To;
							mail.Copy = MSHash(nullptr);
							mail.Date = mailView.Date;
							mail.Type = mailView.Type | OPENMS_MAIL_TYPE_CLIENT;
							mail.Body = MSStringView(mailView.Body, event->Message.size() - sizeof(MailView));

							MS_DEBUG("client %u=>%u via %u #%u @%u", mail.From, mail.To, mail.Copy, mail.Date, mail.Type);
							mailHub->send(mail);
						}
						else
						{
							auto userID = (uint32_t)channel->getContext()->userdata();
							if (userID == 0) return false;
							IMail mail = {};
							mail.From = mailView.From;
							mail.To = mailView.To;
							mail.Copy = MSHash("proxy:" + std::to_string(userID));
							mail.Date = mailView.Date;
							mail.Type = mailView.Type | OPENMS_MAIL_TYPE_FORWARD;
							mail.Body = MSStringView(mailView.Body, event->Message.size() - sizeof(MailView));
							if (mail.Type & OPENMS_MAIL_TYPE_REQUEST) mail.Type |= OPENMS_MAIL_TYPE_CLIENT;
							if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE) mail.Type &= ~OPENMS_MAIL_TYPE_CLIENT;

							MS_INFO("client %u=>%u via %u #%u @%u", mail.From, mail.To, mail.Copy, mail.Date, mail.Type);
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
			},
			.OnClose = [=](MSRef<IChannel> channel)
			{
				auto& guestData = channel->getContext()->attribs()["guestID"];
				auto guestID = std::any_cast<uint32_t>(guestData);
				auto serviceName = "guest:" + std::to_string(guestID);
				mailHub->cancel(serviceName);

				if (auto userID = (uint32_t)channel->getContext()->userdata())
				{
					serviceName = "proxy:" + std::to_string(userID);
					mailHub->cancel(serviceName);
					channel->getContext()->userdata() = 0;
				}
			},
		}
	});
	m_TCPServer->startup();

	m_KeepAlive = startTimer(0, OPENMS_HEARTBEAT * 1000, [this]()
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
	if (!m_KeepAlive.expired()) stopTimer(m_KeepAlive);
	m_KeepAlive.reset();

	if (m_TCPServer) m_TCPServer->shutdown();
	m_TCPServer = nullptr;

	ClusterServer::onExit();
}

bool GatewayServer::onFail(IMail mail)
{
	// Try to send to other domain.

	return false;
}
