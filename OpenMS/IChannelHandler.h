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
#include "MS.h"
#include "IChannelEvent.h"
#include "IChannelContext.h"

/// @brief Interface for inbound handler
class OPENMS_API IChannelInboundHandler
{
public:
	virtual ~IChannelInboundHandler() = default;

	virtual void channelCatch(TRaw<IChannelContext> context, TException&& exception) const = 0;

	virtual void channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const = 0;
};

/// @brief Interface for outbound handler
class OPENMS_API IChannelOutboundHandler
{
public:
	virtual ~IChannelOutboundHandler() = default;

	virtual void channelCatch(TRaw<IChannelContext> context, TException&& exception) const = 0;

	virtual void channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const = 0;
};