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
	void channelCatch(TRaw<IChannelContext> context, TException&& exception) const override;
	void channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};

class ChannelOutboundHandler : public IChannelOutboundHandler
{
public:
	void channelCatch(TRaw<IChannelContext> context, TException&& exception) const override;
	void channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};