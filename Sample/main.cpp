#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/Private/ChannelReactor.h>
#include <OpenMS/Private/ChannelHandler.h>

int main()
{
	ChannelReactor s(4, {
		[](TRef<IChannel> channel) {	// Connected
			auto handler = TNew<ChannelInboundHandler>();
			channel->getPipeline()->addLast("https", handler);
			channel->getPipeline()->addLast("http", handler);
			channel->getPipeline()->addLast("auth", handler);
			channel->getPipeline()->addLast("business", handler);
		},
		[](TRef<IChannel> channel) {	// Disconnected

		},
		});

	s.startup();

	std::this_thread::sleep_for(std::chrono::seconds(1));
	s.shutdown();
	return 0;
}