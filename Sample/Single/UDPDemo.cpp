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

class ServerDemo : public UDPServer
{
protected:
	void configureEndpoint(config_t& config) override
	{
		config.IP = "127.0.0.1";
		config.PortNum = 8080;
		config.Callback.OnOpen = [](MSRef<IChannel> channel)
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
		};
	}
};

class ClientDemo : public UDPClient
{
protected:
	void configureEndpoint(config_t& config) override
	{
		config.IP = "127.0.0.1";
		config.PortNum = 8080;
		config.Callback.OnOpen = [](MSRef<IChannel> channel)
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