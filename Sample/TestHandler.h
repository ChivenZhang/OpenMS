#pragma once
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
#include <OpenMS/Reactor/Private/ChannelHandler.h>

class ServerInboundHandler : public ChannelInboundHandler
{
public:
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) override
	{
		TPrint("Read: %s", event->Message.c_str());
		return true;
	}
};

class ServerOutboundHandler : public ChannelOutboundHandler
{
public:
	bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) override
	{
		auto _event = TNew<IChannelEvent>();
		_event->Message = event->Message;
		context->write(_event);
		return true;
	}
};

class ClientInboundHandler : public ChannelInboundHandler
{
public:
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) override
	{
		TPrint("Read: %s", event->Message.c_str());
		return true;
	}
};

class ClientOutboundHandler : public ChannelOutboundHandler
{
public:
	bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) override
	{
		printf(">>");

		char buffer[1024]{};
		size_t buflen = 0;
		while (scanf("%c", buffer + buflen) != EOF && buffer[buflen] != '\n') ++buflen;

		auto _event = TNew<IChannelEvent>();
		_event->Message = TStringView(buffer, buflen);
		context->write(_event);
		return true;
	}
};