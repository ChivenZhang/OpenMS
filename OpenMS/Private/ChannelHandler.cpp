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

void ChannelInboundHandler::channelCatch(TRaw<IChannelContext> context, TException&& exception) const
{
}

void ChannelInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	printf("read\n");
}

void ChannelOutboundHandler::channelCatch(TRaw<IChannelContext> context, TException&& exception) const
{
}

void ChannelOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
}
