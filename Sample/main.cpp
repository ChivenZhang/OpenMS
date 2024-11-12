#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/TCP/TCPServerReactor.h>
#include <OpenMS/TCP/TCPClientReactor.h>
#include "TestHandler.h"

int main()
{
	system("pause");
	if (true)
	{
		TCPServerReactor server("127.0.0.1", 6000, 128, 4, {
			[](TRef<IChannel> channel) {	// Connected
				auto inbound = TNew<TestInboundHandler>();
				channel->getPipeline()->addFirst("https", inbound);

				auto outbound = TNew<TestOutboundHandler>();
				channel->getPipeline()->addFirst("https", outbound);

				auto event = TNew<IChannelEvent>();
				event->Message = "Hello";
				channel->write(event);
			},
			[](TRef<IChannel> channel) {	// Disconnect
				// Do something here
			},
			});
		server.startup();

		TCPClientReactor client("127.0.0.1", 6000, 1, {
			[](TRef<IChannel> channel) {	// Connected
				auto inbound = TNew<TestInboundHandler>();
				channel->getPipeline()->addFirst("https", inbound);

				auto outbound = TNew<TestOutboundHandler>();
				channel->getPipeline()->addFirst("https", outbound);
			},
			[](TRef<IChannel> channel) {	// Disconnect
				// Do something here
			},
			});
		client.startup();

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	return 0;
}