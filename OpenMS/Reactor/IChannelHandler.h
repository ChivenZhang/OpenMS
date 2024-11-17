#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
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

	virtual void channelError(TRaw<IChannelContext> context, TException&& exception) const = 0;

	virtual bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const = 0;
};

/// @brief Interface for outbound handler
class OPENMS_API IChannelOutboundHandler
{
public:
	virtual ~IChannelOutboundHandler() = default;

	virtual void channelError(TRaw<IChannelContext> context, TException&& exception) const = 0;

	virtual bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const = 0;
};