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
#include "BusinessServer.h"

#include "Handler/AES/AESHandler.h"
#include "Mailbox/Private/Mail.h"
#include "Server/Private/Service.h"

#define AES256_KEY 0x50, 0x71, 0x47, 0x55, 0x6f, 0x4c, 0x36, 0x7a, 0x55, 0x50, 0x38, 0x43, 0x30, 0x42, 0x38, 0x43, 0x2f, 0x55, 0x45, 0x4d, 0x50, 0x49, 0x73, 0x36, 0x4f, 0x63, 0x42, 0x4c, 0x79, 0x32, 0x72, 0x35

MSString BusinessServer::identity() const
{
	return "business";
}

void BusinessServer::onInit()
{
	ClusterServer::onInit();

	m_TCPClient = MSNew<TCPClient>(TCPClient::config_t{
		.IP = "127.0.0.1",
		.PortNum = 9090,
		.Workers = 1,
		.Callback = ChannelReactor::callback_t
		{
			.OnOpen = [](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addFirst("decrypt", MSNew<AESInboundHandler>(AESInboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("input", IChannelPipeline::handler_in
				{
					.OnHandle = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						MS_INFO("客户端接收：%u", (uint32_t)event->Message.size());
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

				MSString content(R"(["guest", "guest"])");
				MSString request(sizeof(MailView) + sizeof(Service::request_t) + content.size(), 0);
				auto& mailView = *(MailView*)request.data();
				mailView.From = 0;
				mailView.To = MSHash("gateway");
				mailView.Date = 0;
				mailView.Type = OPENMS_MAIL_TYPE_REQUEST;
				auto& mailRequest = *(Service::request_t*)mailView.Body;
				mailRequest.Method = MSHash("login");
				::memcpy(mailRequest.Buffer, content.data(), content.size());
				channel->writeChannel(IChannelEvent::New(MSStringView(request)));
			},
		}
	});
	m_TCPClient->startup();
}

void BusinessServer::onExit()
{
	ClusterServer::onExit();

	if (m_TCPClient) m_TCPClient->shutdown();
	m_TCPClient = nullptr;
}