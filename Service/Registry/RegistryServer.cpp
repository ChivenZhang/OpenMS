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
#include "RegistryServer.h"
#include <OpenMS/Reactor/Private/ChannelHandler.h>

struct RegistryServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	OPENMS_TYPE(RegistryServerConfig, ip, port, backlog)
};

RegistryServer::RegistryServer()
{
	startup();
}

RegistryServer::~RegistryServer()
{
	shutdown();
}

void RegistryServer::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE(IProperty)::bean();
	auto configInfo = properties->property<RegistryServerConfig>("registry.server");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;
	config.Callback = {
		[=](TRef<IChannel> channel) {
			auto ipv4 = TCast<IPv4Address>(channel->getRemote());
			TPrint("accept %s:%d", ipv4->getAddress().c_str(), ipv4->getPort());

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
		},
		[=](TRef<IChannel> channel) {
			auto ipv4 = TCast<IPv4Address>(channel->getRemote());
			TPrint("closed %s:%d", ipv4->getAddress().c_str(), ipv4->getPort());
		}
	};
}
