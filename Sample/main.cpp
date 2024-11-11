#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/Private/ChannelReactor.h>
#include "TestHandler.h"

int main()
{
	ChannelReactor s(4, {
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

	s.startup();

	std::this_thread::sleep_for(std::chrono::seconds(1));
	s.shutdown();
	return 0;
}