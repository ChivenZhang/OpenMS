/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Service/IStartup.h>
#include <OpenMS/Reactor/TCP/TCPServerReactor.h>

TMutex lock; TMutexUnlock unlock;

int openms_main(int argc, char** argv)
{
	TVector<TRef<IChannel>> clients;

	TCPServerReactor server(IPv4Address::New("127.0.0.1", 8080), 0, 0, TCPServerReactor::callback_tcp_t{
		[&](TRef<IChannel> channel) {
			TPrint("accept %s", channel->getRemote().lock()->getString().c_str());
			clients.push_back(channel);

			channel->getPipeline()->addFirst("broadcast", {
				.OnRead = [&](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool {
					for (size_t i = 0; i < clients.size(); ++i)
					{
						if (clients[i] == channel) continue;
						auto _event = TNew<IChannelEvent>();
						_event->Message = event->Message;
						clients[i]->write(_event);
					}
					return false;
				}
				});
		},
		[&](TRef<IChannel> channel) {
			TPrint("reject %s", channel->getRemote().lock()->getString().c_str());
			clients.erase(std::remove(clients.begin(), clients.end(), channel), clients.end());
		}
		});

	server.startup();
	signal(SIGINT, [](int) { unlock.notify_all(); });
	TUniqueLock _lock(lock); unlock.wait(_lock);
	server.shutdown();
	return 0;
}
