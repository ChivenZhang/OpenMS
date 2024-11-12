#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/TCP/TCPServerReactor.h>
#include <OpenMS/TCP/TCPClientReactor.h>
#include "TestHandler.h"

int main()
{
	if (true)
	{
		TCPServerReactor server("0.0.0.0", 6000, 128, 4, {
			[](TRef<IChannel> channel) {	// Connected
				auto ipv4 = TCast<IPv4Address>(channel->getRemote());
				TPrint("connect %s:%d", ipv4->getAddress().c_str(), ipv4->getPort());

				auto inbound = TNew<ServerInboundHandler>();
				channel->getPipeline()->addFirst("https", inbound);

				auto outbound = TNew<ServerOutboundHandler>();
				channel->getPipeline()->addFirst("https", outbound);
			},
			[](TRef<IChannel> channel) {	// Disconnect
				auto ipv4 = TCast<IPv4Address>(channel->getRemote());
				TPrint("disconnect %s:%d", ipv4->getAddress().c_str(), ipv4->getPort());
			},
			});
		server.startup();

		TCPClientReactor client("192.168.1.2", 6000, 1, {
			[](TRef<IChannel> channel) {	// Connected
				auto ipv4 = TCast<IPv4Address>(channel->getRemote());
				TPrint("connect %s:%d", ipv4->getAddress().c_str(), ipv4->getPort());

				auto inbound = TNew<ClientInboundHandler>();
				channel->getPipeline()->addFirst("https", inbound);

				auto outbound = TNew<ClientOutboundHandler>();
				channel->getPipeline()->addFirst("https", outbound);

				auto event = TNew<IChannelEvent>();
				event->Message = "Hello";
				channel->write(event);
			},
			[](TRef<IChannel> channel) {	// Disconnect
				auto ipv4 = TCast<IPv4Address>(channel->getRemote());
				TPrint("disconnect %s:%d", ipv4->getAddress().c_str(), ipv4->getPort());
			},
			});
		client.startup();

		TMutex mutex;
		TMutexUnlock unlock;
		TUniqueLock lock(mutex);
		unlock.wait(lock);
	}
	return 0;
}