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
#include "FrameClient.h"
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

			TArray<uint8_t, 16> iv;
			TArray<uint8_t, 32> key;
			memcpy(iv.data(), OPENMS_AES256_IV, 16);
			memcpy(key.data(), OPENMS_AES256_KEY, 32);
			channel->getPipeline()->addLast("decrypt", TNew<AESInboundHandler>(AESInboundHandler::config_t{ key, iv}));

			channel->getPipeline()->addLast("read", {
				.OnRead = [=](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) {

					sync::Message msg;
					if (msg.ParseFromString(event->Message))
					{
						auto service = TCast<FrameService>(AUTOWIRE(IService)::bean());
						service->sendEvent([service, msg]() mutable { service->onMessage(std::move(msg)); });
					}
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