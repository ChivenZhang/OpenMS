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
#include "IChannelHandler.h"

class ChannelInboundHandler : public IChannelInboundHandler
{
public:
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};

class ChannelOutboundHandler : public IChannelOutboundHandler
{
public:
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};