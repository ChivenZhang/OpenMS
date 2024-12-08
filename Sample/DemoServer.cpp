#include "DemoServer.h"
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/

void DemoServer::configureEndpoint(config_t& config)
{
	config.IP = "0.0.0.0";
	config.PortNum = 8080;
	config.Callback =
	{
		.OnOpen = [](MSRef<IChannel> channel) {

			channel->getPipeline()->addLast("read", {
				.OnRead = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
				{
					MS_PRINT("Server read: %s", event->Message.c_str());
					return false;
				}
				});

			channel->getPipeline()->addLast("send", {
				.OnWrite = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
				{
					context->write(IChannelEvent::New(event->Message));
					return false;
				}
				});
		},

		.OnClose = [](MSRef<IChannel> channel) {
			MS_PRINT("rejected %s", channel->getRemote().lock()->getString().c_str());
		},
	};
}

void DemoClient::configureEndpoint(config_t& config)
{
	config.IP = "127.0.0.1";
	config.PortNum = 8080;
	config.Callback =
	{
		.OnOpen = [=](MSRef<IChannel> channel) {

			auto service = AUTOWIRE(IService)::bean();
			service->startTimer(1000, 1000, [=](uint32_t handle) {
				channel->write(IChannelEvent::New("Hello, world!"));
				});
		}
	};
}
