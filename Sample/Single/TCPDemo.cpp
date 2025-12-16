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
#include <OpenMS/Endpoint/TCP/TCPServer.h>
#include <OpenMS/Endpoint/TCP/TCPClient.h>

#include "Utility/QuickQPS.h"
QuickQPS qps;
auto T = ::clock();

int main()
{
	auto server = MSNew<TCPServer>(TCPServer::config_t{
		.IP = "127.0.0.1",
		.PortNum = 8080,
		.Callback = {
			.OnOpen = [](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addFirst("handler", IChannelPipeline::handler_in
				{
					.OnHandle = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
					{
						qps.hit();
						auto t = ::clock();
						if (T + 5000 <= t)
						{
							MS_INFO("QPS %f", qps.get());
							T = t;
						}

						MS_DEBUG("%s", event->Message.c_str());
						context->write(IChannelEvent::New("This is server"));
						return true;
					}
				});
			},
		},
	});
	auto client = MSNew<TCPClient>(TCPClient::config_t{
		.IP = "127.0.0.1",
		.PortNum = 8080,
		.Callback = {
			.OnOpen = [](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addFirst("handler", IChannelPipeline::handler_in
				{
					.OnHandle = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
					{
						MS_DEBUG("%s", event->Message.c_str());
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