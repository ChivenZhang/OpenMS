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
#include "ChannelHandler.h"
#include "IChannelContext.h"

void ChannelInboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception) const
{
}

bool ChannelInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	return false;
}

void ChannelOutboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception) const
{
}

bool ChannelOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	return false;
}
