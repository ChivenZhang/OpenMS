/*=================================================
* Copyright © 2020-2026 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Endpoint/WS/WSServer.h>
#include <OpenMS/Endpoint/WS/WSClient.h>
#include <OpenMS/Reactor/WS/WSChannelEvent.h>

int main()
{
	auto server = MSNew<WSServer>(WSServer::config_t{
		.IP = "127.0.0.1",
		.PortNum = 8080,
		.Callback = {
			.OnOpen = [](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addFirst("handler", IChannelPipeline::handler_in
				{
					.OnHandle = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
					{
						MS_INFO("%s", event->Message.c_str());
						context->write(WSChannelEvent::New("This is server", WSChannelEvent::opcode_t::TEXT));
						return true;
					}
				});
			},
		},
	});
	auto client = MSNew<WSClient>(WSClient::config_t{
		.IP = "127.0.0.1",
		.PortNum = 8080,
		.Path = "/client",
		.Callback = {
			.OnOpen = [](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addFirst("handler", IChannelPipeline::handler_in
				{
					.OnHandle = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
					{
						MS_INFO("%s", event->Message.c_str());
						context->write(WSChannelEvent::New("This is client", WSChannelEvent::opcode_t::TEXT));
						return true;
					}
				});
				channel->write(WSChannelEvent::New("Hello, server!", WSChannelEvent::opcode_t::TEXT));
			},
		},
	});

	server->startup();
	client->startup();

	system("pause");

	server->shutdown();
	client->shutdown();
	return 0;
}
