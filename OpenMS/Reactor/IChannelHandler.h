#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
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

	virtual bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) = 0;

	virtual void channelError(MSRaw<IChannelContext> context, MSError&& exception) = 0;
};

/// @brief Interface for outbound handler
class OPENMS_API IChannelOutboundHandler
{
public:
	virtual ~IChannelOutboundHandler() = default;

	virtual bool channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) = 0;

	virtual void channelError(MSRaw<IChannelContext> context, MSError&& exception) = 0;
};