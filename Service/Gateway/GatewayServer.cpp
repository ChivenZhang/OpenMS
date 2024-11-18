/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "GatewayServer.h"
#include <OpenMS/Reactor/Private/ChannelHandler.h>

struct GatewayServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	uint32_t workers;
	OPENMS_TYPE(GatewayServerConfig, ip, port, backlog, workers)
};

GatewayServer::GatewayServer()
{
	startup();
}

GatewayServer::~GatewayServer()
{
	shutdown();
}

void GatewayServer::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE_DATA(IProperty);
	auto configInfo = properties->property<GatewayServerConfig>("gateway.server");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;
	config.WorkerNum = 1; // single thread for safety
	config.Callback = {
		[=](TRef<IChannel> channel) {
			channel->getPipeline()->addFirst("", IChannelInboundHandler::callback_t{
				[=](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool {
					TPRINT("read %s", event->Message.c_str());
					return false;
				}
				});
			channel->getPipeline()->addFirst("", IChannelOutboundHandler::callback_t{
				[=](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool {
					auto _event = TNew<IChannelEvent>();
					_event->Message = "Echo " + event->Message;
					context->write(_event);
					return false;
				}
				});
		}
	};
}
