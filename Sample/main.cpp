#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/Private/ChannelReactor.h>
#include <OpenMS/TCP/TCPServerReactor.h>
#include "TestHandler.h"

int main()
{
	if (true)
	{
		TCPServerReactor R("127.0.0.1", 6000, 128, 4, {
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

		R.startup();

		std::this_thread::sleep_for(std::chrono::seconds(1));
		R.shutdown();
	}
	return 0;
}