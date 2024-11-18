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
#include "RegistryHandler.h"

bool RegistryInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	TPRINT("read %s", event->Message.c_str());
	return false;
}

bool RegistryOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	auto _event = TNew<IChannelEvent>();
	_event->Message = "Echo " + event->Message;
	context->write(_event);
	return false;
}