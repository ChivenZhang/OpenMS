#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
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

	virtual MSHnd<IChannelAddress> getLocal() const = 0;

	virtual MSHnd<IChannelAddress> getRemote() const = 0;

	virtual MSRaw<IChannelContext> getContext() const = 0;

	virtual MSRaw<IChannelPipeline> getPipeline() const = 0;

	virtual void readChannel(MSRef<IChannelEvent> event) = 0;

	virtual void writeChannel(MSRef<IChannelEvent> event) = 0;

	virtual void close() = 0;

	virtual MSFuture<bool> close(MSPromise<bool>& promise) = 0;

	virtual void write(MSRef<IChannelEvent> event) = 0;

	virtual MSFuture<bool> write(MSRef<IChannelEvent> event, MSPromise<bool>& promise) = 0;

	virtual void writeAndFlush(MSRef<IChannelEvent> event) = 0;

	virtual MSFuture<bool> writeAndFlush(MSRef<IChannelEvent> event, MSPromise<bool>& promise) = 0;
};