#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include <OpenMS/Private/ChannelHandler.h>

class TestInboundHandler : public ChannelInboundHandler
{
public:
	void channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override
	{
		TPrint("Read: %s\n", event->Message.c_str());
	}
};

class TestOutboundHandler : public ChannelOutboundHandler
{
public:
	void channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override
	{
		TPrint("Write: %s\n", event->Message.c_str());

		auto _event = TNew<IChannelEvent>(IChannelEvent{ "A message!" });
		context->write(_event);
	}
};