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

	TCPServerReactor timeSync(IPv4Address::New("0.0.0.0", 9090), 0, 0, {
		[&](TRef<IChannel> channel) {
			TPrint("accept %s", channel->getRemote().lock()->getString().c_str());

			channel->getPipeline()->addFirst("timesync", {
				.OnRead = [&, channel](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) -> bool {
					auto t2 = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
					decltype(t2) buffer[2]{ t2, t2 };
					context->writeAndFlush(IChannelEvent::New(TString((char*)buffer, sizeof(buffer))));
					return false;
				}
				});
		}
		});
	timeSync.startup();

	TCPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 0, 0, {
		[&](TRef<IChannel> channel) {
			TPrint("accept %s", channel->getRemote().lock()->getString().c_str());
			clients.push_back(channel);

			TString buffer;
			channel->getPipeline()->addFirst("broadcast", {
				.OnRead = [&, channel](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) mutable ->bool {
					for (size_t i = 0; i < clients.size(); ++i)
					{
						if (clients[i] == channel) continue;
						auto _event = TNew<IChannelEvent>();
						_event->Message = event->Message;
						clients[i]->writeAndFlush(_event);
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

	timeSync.shutdown();
	return 0;
}
