#include <iostream>
#include <OpenMS/Reactor/TCP/TCPServerReactor.h>
#include <OpenMS/Reactor/TCP/TCPClientReactor.h>
#include <OpenMS/Reactor/UDP/UDPServerReactor.h>
#include <OpenMS/Reactor/UDP/UDPClientReactor.h>
#include <OpenMS/Reactor/KCP/KCPServerReactor.h>
#include <OpenMS/Reactor/KCP/KCPClientReactor.h>
#include <OpenMS/Reactor/Private/ChannelHandler.h>

#define SERVER_HANDLER \
	[](TRef<IChannel> channel) { \
		auto ip = TCast<IPv4Address>(channel->getRemote().lock()); \
		TPrint("connect %s", ip->getString().c_str()); \
		\
		channel->getPipeline()->addFirst("input", { \
			.OnRead = [](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool \
			{ \
				TPrint("Server read: %s", event->Message.c_str()); \
				return true; \
			} \
			}); \
		\
		channel->getPipeline()->addFirst("output", { \
			.OnWrite = [](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool \
			{ \
				context->write(IChannelEvent::New(event->Message)); \
				return true; \
			} \
			}); \
	}, \
	[](TRef<IChannel> channel) { \
		auto ip = TCast<IPv4Address>(channel->getRemote().lock()); \
		TPrint("disconnect %s", ip->getString().c_str()); \
	},

#define CLIENT_HANDLER \
	[](TRef<IChannel> channel) { \
		auto ip = TCast<IPv4Address>(channel->getRemote().lock()); \
		TPrint("connect %s", ip->getString().c_str()); \
		\
		channel->getPipeline()->addFirst("input", { \
			.OnRead = [](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool \
			{ \
				TPrint("Client read: %s", event->Message.c_str()); \
				return true; \
			} \
			}); \
		\
		channel->getPipeline()->addFirst("output", { \
			.OnWrite = [](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool \
			{ \
				printf(">>"); \
				\
				char buffer[1024]{}; \
				size_t buflen = 0; \
				while (scanf("%c", buffer + buflen) != EOF && buffer[buflen] != '\n') ++buflen; \
				\
				context->write(IChannelEvent::New(TStringView(buffer, buflen))); \
				return true; \
			} \
			}); \
		channel->write(IChannelEvent::New("Hello, server!\n")); \
	}, \
	[](TRef<IChannel> channel) { \
		auto ip = TCast<IPv4Address>(channel->getRemote().lock()); \
		TPrint("disconnect %s", ip->getString().c_str()); \
	},

int main()
{
	if (true)
	{
		TCPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 128, 0, { SERVER_HANDLER });
		server.startup();

		TCPClientReactor client(IPv4Address::New("127.0.0.1", 8080), 0, { CLIENT_HANDLER });
		client.startup();

		TMutex mutex; TMutexUnlock unlock; TUniqueLock lock(mutex); unlock.wait(lock);
	}

	if (false)
	{
		UDPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 0, false, false, 0, { SERVER_HANDLER });
		server.startup();

		UDPClientReactor client(IPv4Address::New("127.0.0.1", 8080), false, false, 0, { CLIENT_HANDLER });
		client.startup();

		TMutex mutex; TMutexUnlock unlock; TUniqueLock lock(mutex); unlock.wait(lock);
	}

	if (false)
	{
		KCPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 0, 0, { SERVER_HANDLER });
		server.startup();

		KCPClientReactor client(IPv4Address::New("127.0.0.1", 8080), 0, { CLIENT_HANDLER });
		client.startup();

		TMutex mutex; TMutexUnlock unlock; TUniqueLock lock(mutex); unlock.wait(lock);
	}

	return 0;
}