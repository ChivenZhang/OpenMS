#include <iostream>
#include <OpenMS/IService.h>
#include <OpenMS/TCP/TCPServerReactor.h>
#include <OpenMS/TCP/TCPClientReactor.h>
#include <OpenMS/UDP/UDPServerReactor.h>
#include <OpenMS/UDP/UDPClientReactor.h>
#include <OpenMS/KCP/KCPServerReactor.h>
#include <OpenMS/KCP/KCPClientReactor.h>
#include "TestHandler.h"

int main()
{
	if (false)
	{
		TCPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 128, 2, {
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

		TCPClientReactor client(IPv4Address::New("127.0.0.1", 8080), 1, {
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

		TMutex mutex; TMutexUnlock unlock; TUniqueLock lock(mutex); unlock.wait(lock);
	}

	if (false)
	{
		UDPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 0, false, false, 2, {
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

		UDPClientReactor client(IPv4Address::New("127.0.0.1", 8080), false, false, 1, {
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

		TMutex mutex; TMutexUnlock unlock; TUniqueLock lock(mutex); unlock.wait(lock);
	}

	if (true)
	{
		KCPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 1, 2, {
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

		KCPClientReactor client(IPv4Address::New("127.0.0.1", 8080), 1, {
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

		TMutex mutex; TMutexUnlock unlock; TUniqueLock lock(mutex); unlock.wait(lock);
	}

	return 0;
}