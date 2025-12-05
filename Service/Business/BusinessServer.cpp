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
	static uint32_t s_UserID = 0;
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	auto logicService = MSNew<Service>();
	logicService->bind("login", [=, this](MSString user, MSString pass)->MSAsync<uint32_t>
	{
		auto userID = ++s_UserID;

		auto playerService = MSNew<Service>();
		playerService->bind("attack", []()->MSAsync<void>
		{
			MS_INFO("BiuBiu!!!");
			co_return;
		});
		if (mailHub->create("player:" + std::to_string(userID), playerService)) this->onPush();
		co_return userID;
	});
	logicService->bind("logout", [](uint32_t userID)->MSAsync<uint32_t>
	{
		co_return userID < s_UserID;
	});
	logicService->bind("signup", [](MSString user, MSString pass)->MSAsync<bool>
	{
		co_return true;
	});
	if (mailHub->create("logic", logicService)) this->onPush();

	MSRef<IChannel> clientChannel;

	m_TCPClient = MSNew<TCPClient>(TCPClient::config_t{
		.IP = "127.0.0.1",
		.PortNum = 9090,
		.Workers = 1,
		.Callback = ChannelReactor::callback_t
		{
			.OnOpen = [&clientChannel, this](MSRef<IChannel> channel)
			{
				clientChannel = channel;

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

				MSString content(R"(["admin", "123456"])");
				MSString request(sizeof(MailView) + sizeof(Service::request_t) + content.size(), 0);
				auto& mailView = *(MailView*)request.data();
				mailView.From = 0;
				mailView.To = MSHash("gateway");
				mailView.Date = 0;
				mailView.Type = OPENMS_MAIL_TYPE_REQUEST | OPENMS_MAIL_TYPE_CLIENT;
				auto& mailRequest = *(Service::request_t*)mailView.Body;
				mailRequest.Method = MSHash("login");
				::memcpy(mailRequest.Buffer, content.data(), content.size());
				channel->writeChannel(IChannelEvent::New(MSStringView(request)));
			},
		}
	});
	m_TCPClient->startup();

	this->startTimer(4000, 0, [=](uint32_t handle)
	{
		MSString content(R"()");
		MSString request(sizeof(MailView) + sizeof(Service::request_t) + content.size(), 0);
		auto& mailView = *(MailView*)request.data();
		mailView.From = 0;
		mailView.To = MSHash("player:1");
		mailView.Date = 0;
		mailView.Type = OPENMS_MAIL_TYPE_REQUEST | OPENMS_MAIL_TYPE_CLIENT;
		auto& mailRequest = *(Service::request_t*)mailView.Body;
		mailRequest.Method = MSHash("attack");
		if (content.empty() == false) ::memcpy(mailRequest.Buffer, content.data(), content.size());
		clientChannel->writeChannel(IChannelEvent::New(MSStringView(request)));
	});
}

void BusinessServer::onExit()
{
	ClusterServer::onExit();

	if (m_TCPClient) m_TCPClient->shutdown();
	m_TCPClient = nullptr;
}