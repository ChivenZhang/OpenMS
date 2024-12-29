/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "FrameServer.h"
#include "FrameService.h"
#include "../Message.pb.h"
#include <OpenMS/Handler/AES/AESHandler.h>

#define OPENMS_AES256_IV "y8WOkCzXZHmRLMq6"
#define OPENMS_AES256_KEY "6BGtsnEW9s2QJalfTHFcXUi46JgJmmDe"

void FrameServer::configureEndpoint(config_t& config) const
{
	config.IP = "0.0.0.0";
	config.PortNum = 8080;
	config.Callback =
	{
		.OnOpen = [=](MSRef<IChannel> channel) {

			// Read channel

			FramePackage buffer;
			channel->getPipeline()->addLast("buffer", {
				.OnRead = [=](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) mutable {
					buffer.Data += event->Message;
					if (buffer.Data.size() < sizeof(uint32_t)) return false;
					buffer.Size = *(uint32_t*)buffer.Data.data();
					if (buffer.Data.size() < sizeof(uint32_t) + buffer.Size) return false;
					event->Message = buffer.Data.substr(sizeof(uint32_t), buffer.Size);
					buffer.Data = buffer.Data.substr(sizeof(uint32_t) + buffer.Size);
					return true;
					},
				});

			MSArray<uint8_t, 32> key;
			MSArray<uint8_t, 16> iv;
			memcpy(iv.data(), OPENMS_AES256_IV, 16);
			memcpy(key.data(), OPENMS_AES256_KEY, 32);
			channel->getPipeline()->addLast("decrypt", MSNew<AESInboundHandler>(AESInboundHandler::config_t{ key, iv}));

			channel->getPipeline()->addLast("read", {
				.OnRead = [=](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) {

					sync::Message msg;
					if (msg.ParseFromString(event->Message))
					{
						auto service = MSCast<FrameService>(AUTOWIRE(IService)::bean());
						service->sendEvent([service, msg]() mutable { service->onMessage(std::move(msg)); });
					}
					return false;
					},
				});

			// Write channel

			channel->getPipeline()->addLast("encrypt", MSNew<AESOutboundHandler>(AESOutboundHandler::config_t{ key, iv }));

			channel->getPipeline()->addLast("send", {
				.OnWrite = [=](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) mutable {
					buffer.Size = (uint32_t)event->Message.size();
					buffer.Data.resize(sizeof(uint32_t) + event->Message.size());
					memcpy(buffer.Data.data(), &buffer.Size, sizeof(uint32_t));
					memcpy(buffer.Data.data() + sizeof(uint32_t), event->Message.data(), event->Message.size());

					context->write(IChannelEvent::New(buffer.Data));
					return false;
					}
				});
		}
	};
}