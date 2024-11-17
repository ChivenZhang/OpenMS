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
#include "IChannelAddress.h"
#include "IChannelContext.h"
#include "IChannelPipeline.h"

/// @brief Interface for channel
class OPENMS_API IChannel
{
public:
	virtual ~IChannel() = default;

	virtual bool running() const = 0;

	virtual uint32_t getWorkID() const = 0;

	virtual TRaw<IChannelAddress> getLocal() const = 0;

	virtual TRaw<IChannelAddress> getRemote() const = 0;

	virtual TRaw<IChannelContext> getContext() const = 0;

	virtual TRaw<IChannelPipeline> getPipeline() const = 0;

	virtual void close() = 0;

	virtual TFuture<bool> close(TPromise<bool>&& promise) = 0;

	virtual void read(TRef<IChannelEvent> event) = 0;

	virtual TFuture<bool> read(TRef<IChannelEvent> event, TPromise<bool>&& promise) = 0;

	virtual void write(TRef<IChannelEvent> event) = 0;

	virtual TFuture<bool> write(TRef<IChannelEvent> event, TPromise<bool>&& promise) = 0;

	virtual void writeAndFlush(TRef<IChannelEvent> event) = 0;

	virtual TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TPromise<bool>&& promise) = 0;
};