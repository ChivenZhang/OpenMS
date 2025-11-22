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

#include "Endpoint/TCP/TCPServer.h"
#include "Handler/AES/AESHandler.h"
#include "Server/Private/Service.h"

#define AES256_KEY 0x50, 0x71, 0x47, 0x55, 0x6f, 0x4c, 0x36, 0x7a, 0x55, 0x50, 0x38, 0x43, 0x30, 0x42, 0x38, 0x43, 0x2f, 0x55, 0x45, 0x4d, 0x50, 0x49, 0x73, 0x36, 0x4f, 0x63, 0x42, 0x4c, 0x79, 0x32, 0x72, 0x35

// ========================================================================================

MSString GatewayServer::identity() const
{
	return "gateway";
}

void GatewayServer::onInit()
{
	ClusterServer::onInit();

	auto hub = AUTOWIRE(IMailHub)::bean();
	auto servce = MSNew<Service>();
	hub->create("gateway", servce);

	m_TCPServer = MSNew<TCPServer>(TCPServer::config_t{
		.IP = property(identity() + ".client.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".client.port", 0U),
		.Backlog = property(identity() + ".client.backlog", 0U),
		.Workers = property(identity() + ".client.workers", 0U),
		.Callback = ChannelReactor::callback_t
		{
			.OnOpen = [=](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addFirst("decrypt", MSNew<AESInboundHandler>(AESInboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("input", IChannelPipeline::handler_in
				{
					.OnHandle = [=](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						// TODO: Auth token verify here
						servce->call<void>("world", "entry", 0, MSTuple{ event->Message });
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
		}
	});
	m_TCPServer->startup();

	m_Heartbeat2 = startTimer(0, OPENMS_HEARTBEAT * 1000, [this](uint32_t handle)
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
	if (m_Heartbeat2) stopTimer(m_Heartbeat2);
	m_Heartbeat2 = 0;

	if (m_TCPServer) m_TCPServer->shutdown();
	m_TCPServer = nullptr;

	ClusterServer::onExit();
}