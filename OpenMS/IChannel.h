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
#include "IChannelAddress.h"
#include "IChannelContext.h"
#include "IChannelPipeline.h"

/// @brief Interface for channel
class OPENMS_API IChannel
{
public:
	virtual ~IChannel() = default;

	virtual bool running() const = 0;

	virtual TRaw<const IChannelAddress> getLocal() const = 0;

	virtual TRaw<const IChannelAddress> getRemote() const = 0;

	virtual TRaw<const IChannelContext> getContext() const = 0;

	virtual TRaw<IChannelPipeline> getPipeline() = 0;

	virtual TRaw<const IChannelPipeline> getPipeline() const = 0;

	virtual void close() = 0;

	virtual TFuture<bool> close(TPromise<bool>&& promise) = 0;

	virtual void read(TRaw<IChannelEvent> event) = 0;

	virtual TFuture<bool> read(TRaw<IChannelEvent> event, TPromise<bool>&& promise) = 0;

	virtual void write(TRaw<IChannelEvent> event) = 0;

	virtual TFuture<bool> write(TRaw<IChannelEvent> event, TPromise<bool>&& promise) = 0;
};