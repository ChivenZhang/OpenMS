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
#include "FrameServer.h"
#include "FrameService.h"
#include "../Message.pb.h"
#include <OpenMS/Handler/AES/AESHandler.h>

void FrameClient::configureEndpoint(config_t& config)
{
	config.IP = "127.0.0.1";
	config.PortNum = 8080;
	config.Callback =
	{
		.OnOpen = [=](TRef<IChannel> channel) {

			// Read channel

			FramePackage buffer;
			channel->getPipeline()->addLast("buffer", {
				.OnRead = [=](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) mutable {
					buffer.Data += event->Message;
					if (buffer.Data.size() < sizeof(uint32_t)) return false;
					buffer.Size = *(uint32_t*)buffer.Data.data();
					if (buffer.Data.size() < sizeof(uint32_t) + buffer.Size) return false;
					event->Message = buffer.Data.substr(sizeof(uint32_t), buffer.Size);
					buffer.Data = buffer.Data.substr(sizeof(uint32_t) + buffer.Size);
					return true;
					},
				});

			TArray<uint8_t, 32> key;
			TArray<uint8_t, 16> iv;
			memcpy(iv.data(), OPENMS_AES256_IV, 16);
			memcpy(key.data(), OPENMS_AES256_KEY, 32);
			channel->getPipeline()->addLast("decrypt", TNew<AESInboundHandler>(AESInboundHandler::config_t{ key, iv}));

			channel->getPipeline()->addLast("read", {
				.OnRead = [=](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) {
					TPrint("client read %s", event->Message.c_str());
					return false;
					},
				});

			// Write channel

			channel->getPipeline()->addLast("encrypt", TNew<AESOutboundHandler>(AESOutboundHandler::config_t{ key, iv }));

			channel->getPipeline()->addLast("send", {
				.OnWrite = [=](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) mutable {
					buffer.Size = (uint32_t)event->Message.size();
					buffer.Data.resize(sizeof(uint32_t) + event->Message.size());
					memcpy(buffer.Data.data(), &buffer.Size, sizeof(uint32_t));
					memcpy(buffer.Data.data() + sizeof(uint32_t), event->Message.data(), event->Message.size());

					context->write(IChannelEvent::New(buffer.Data));
					return false;
					}
				});

			sync::Message msg;
			msg.set_type(sync::MSG_ENTER_BATTLE);
			sync::MsgEnterBattle enter_battle;
			msg.mutable_body()->mutable_enter_battle()->set_name("Hello, Server!");

			TString result;
			msg.SerializeToString(&result);
			channel->writeChannel(IChannelEvent::New(result));
		}
	};
}

//
//int main(int argc, char* argv[])
//{
//	TVector<TRef<IChannel>> clients;
//
//	TCPServerReactor timeSync(IPv4Address::New("0.0.0.0", 9090), 0, 0, {
//		[&](TRef<IChannel> channel) {
//			channel->getPipeline()->addFirst("timesync", {
//				.OnRead = [&, channel](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) -> bool {
//					auto t2 = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
//					decltype(t2) buffer[2]{ t2, t2 };
//					context->writeAndFlush(IChannelEvent::New(TString((char*)buffer, sizeof(buffer))));
//					return false;
//				}
//				});
//		}
//		});
//	timeSync.startup();
//
//	TCPServerReactor server(IPv4Address::New("0.0.0.0", 8080), 0, 0, {
//		[&](TRef<IChannel> channel) {
//			TPrint("accept %s", channel->getRemote().lock()->getString().c_str());
//			clients.push_back(channel);
//
//			channel->getPipeline()->addFirst("broadcast", {
//				.OnRead = [&, channel](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) mutable ->bool {
//					for (size_t i = 0; i < clients.size(); ++i)
//					{
//						if (clients[i] == channel) continue;
//						auto _event = TNew<IChannelEvent>();
//						_event->Message = event->Message;
//						clients[i]->writeAndFlush(_event);
//					}
//					return false;
//				}
//				});
//		},
//		[&](TRef<IChannel> channel) {
//			TPrint("reject %s", channel->getRemote().lock()->getString().c_str());
//			clients.erase(std::remove(clients.begin(), clients.end(), channel), clients.end());
//		}
//		});
//
//	server.startup();
//	signal(SIGINT, [](int) { unlock.notify_all(); });
//	TUniqueLock _lock(lock); unlock.wait(_lock);
//	server.shutdown();
//
//	timeSync.shutdown();
//	return 0;
//}