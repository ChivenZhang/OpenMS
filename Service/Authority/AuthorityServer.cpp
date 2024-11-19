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
#include "AuthorityServer.h"
#include <OpenMS/Reactor/Private/ChannelHandler.h>

struct AuthorityServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	OPENMS_TYPE(AuthorityServerConfig, ip, port, backlog)
};

AuthorityServer::AuthorityServer()
{
	startup();
}

AuthorityServer::~AuthorityServer()
{
	shutdown();
}

void AuthorityServer::configureEndpoint(config_t& config)
{
	auto properties = AUTOWIRE(IProperty)::bean();
	auto configInfo = properties->property<AuthorityServerConfig>("authority.server");
	config.IP = configInfo.ip;
	config.PortNum = configInfo.port;
	config.Backlog = configInfo.backlog;
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
