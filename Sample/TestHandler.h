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

class ServerInboundHandler : public ChannelInboundHandler
{
public:
	void channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override
	{
		TPrint("Read: %s", event->Message.c_str());
	}
};

class ServerOutboundHandler : public ChannelOutboundHandler
{
public:
	void channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override
	{
		printf(">>");

		auto _event = TNew<IChannelEvent>();
		_event->Message = event->Message;
		context->write(_event);
		TPrint("Write: %s", _event->Message.c_str());
	}
};

class ClientInboundHandler : public ChannelInboundHandler
{
public:
	void channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override
	{
		TPrint("Read: %s", event->Message.c_str());
	}
};

class ClientOutboundHandler : public ChannelOutboundHandler
{
public:
	void channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override
	{
		printf(">>");

		char buffer[1024]{};
		size_t buflen = 0;
		while (scanf("%c", buffer + buflen) != EOF && buffer[buflen] != '\n') ++buflen;

		auto _event = TNew<IChannelEvent>();
		_event->Message = TStringView(buffer, buflen);
		context->write(_event);
		TPrint("Write: %s", _event->Message.c_str());
	}
};