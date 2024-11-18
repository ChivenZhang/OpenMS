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

class GatewayInboundHandler : public ChannelInboundHandler
{
public:
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};

class GatewayOutboundHandler : public ChannelOutboundHandler
{
public:
	bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};