/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Endpoint/UDP/UDPServer.h>
#include <OpenMS/Endpoint/UDP/UDPClient.h>

int main()
{
	auto server = MSNew<UDPServer>(UDPServer::config_t{
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
						context->write(IChannelEvent::New("This is server"));
						return true;
					}
				});
			},
		},
	});
	auto client = MSNew<UDPClient>(UDPClient::config_t{
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
						context->write(IChannelEvent::New("This is client"));
						return true;
					}
				});
				channel->write(IChannelEvent::New("Hello, server!"));
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