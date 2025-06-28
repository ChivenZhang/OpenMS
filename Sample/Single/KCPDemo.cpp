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
#include <OpenMS/Endpoint/KCP/KCPServer.h>
#include <OpenMS/Endpoint/KCP/KCPClient.h>

class ServerDemo : public KCPServer
{
protected:
	void configureEndpoint(config_t& config) override
	{
		config.IP = "127.0.0.1";
		config.PortNum = 8080;
		config.Callback.OnOpen = [](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("handler", IChannelPipeline::inconfig_t
			{
				.OnRead = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
				{
					MS_INFO("%s", event->Message.c_str());
					context->write(IChannelEvent::New("This is server"));
					return true;
				}
			});
		};
	}
};

class ClientDemo : public KCPClient
{
protected:
	void configureEndpoint(config_t& config) override
	{
		config.IP = "127.0.0.1";
		config.PortNum = 8080;
		config.Callback.OnOpen = [](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("handler", IChannelPipeline::inconfig_t
			{
				.OnRead = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
				{
					MS_INFO("%s", event->Message.c_str());
					auto event2 = IChannelEvent::New("This is client");
					context->write(event2);
					return true;
				}
			});

			channel->write(IChannelEvent::New("Hello, server!"));
		};
	}
};

int main()
{
	auto server = MSNew<ServerDemo>();
	auto client = MSNew<ClientDemo>();

	server->startup();
	client->startup();

	system("pause");

	server->shutdown();
	client->shutdown();
	return 0;
}