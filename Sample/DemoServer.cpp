#include "DemoServer.h"
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

void DemoServer::configureEndpoint(config_t& config)
{
	config.IP = "0.0.0.0";
	config.PortNum = 8080;
	config.Callback =
	{
		.OnOpen = [](TRef<IChannel> channel) {
			auto ip = TCast<IPv4Address>(channel->getRemote().lock());
			TPrint("accept %s", ip->getString().c_str());

			channel->getPipeline()->addFirst("read", {
				.OnRead = [](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool
				{
					TPrint("Server read: %s", event->Message.c_str());
					return false;
				}
				});

			channel->getPipeline()->addFirst("send", {
				.OnWrite = [](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool
				{
					context->write(IChannelEvent::New(event->Message));
					return false;
				}
				});
		},

		.OnClose = [](TRef<IChannel> channel) {
			auto ip = TCast<IPv4Address>(channel->getRemote().lock());
			TPrint("reject %s", ip->getString().c_str());
		},
	};
}

void DemoClient::configureEndpoint(config_t& config)
{
	config.IP = "127.0.0.1";
	config.PortNum = 8080;
	config.Callback =
	{
		.OnOpen = [=](TRef<IChannel> channel) {
			auto service = AUTOWIRE(IService)::bean();
			service->startTimer(0, 1000, [=](uint32_t handle) {
				channel->write(IChannelEvent::New("Hello, world!"));
				});
		}
	};
}
