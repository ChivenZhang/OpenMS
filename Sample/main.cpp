#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/Private/ChannelReactor.h>
#include <OpenMS/Private/ChannelHandler.h>

int main()
{
	ChannelReactor s(4, {
		[](TRef<IChannel> channel) {	// Connected
			auto handler = TNew<ChannelInboundHandler>();
			channel->getPipeline()->addFirst("https", handler);
			channel->getPipeline()->addAfter("https", "http", handler);
			channel->getPipeline()->addAfter("http", "auth", handler);
			channel->getPipeline()->addLast("handler1", handler);
			channel->getPipeline()->addBefore("handler1", "handler0", handler);
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