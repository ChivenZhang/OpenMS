#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/TCP/TCPServerReactor.h>
#include <OpenMS/TCP/TCPClientReactor.h>
#include "TestHandler.h"

int main()
{
	if (true)
	{
		TCPServerReactor server("127.0.0.1", 6000, 128, 4, {
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

		server.startup();
		client.startup();

		std::this_thread::sleep_for(std::chrono::seconds(2));
		server.shutdown();
		client.shutdown();
	}
	return 0;
}