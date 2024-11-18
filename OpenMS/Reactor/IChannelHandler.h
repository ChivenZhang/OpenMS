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
#include "MS.h"
#include "IChannelEvent.h"
#include "IChannelContext.h"

/// @brief Interface for inbound handler
class OPENMS_API IChannelInboundHandler
{
public:
	struct callback_t
	{
		TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> OnRead;
		TLambda<void(TRaw<IChannelContext> context, TException&& exception)> OnError;
	};

public:
	virtual ~IChannelInboundHandler() = default;

	virtual void channelError(TRaw<IChannelContext> context, TException&& exception) const = 0;

	virtual bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const = 0;
};

/// @brief Interface for outbound handler
class OPENMS_API IChannelOutboundHandler
{
public:
	struct callback_t
	{
		TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> OnWrite;
		TLambda<void(TRaw<IChannelContext> context, TException&& exception)> OnError;
	};

public:
	virtual ~IChannelOutboundHandler() = default;

	virtual void channelError(TRaw<IChannelContext> context, TException&& exception) const = 0;

	virtual bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const = 0;
};