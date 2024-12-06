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
#include "IChannel.h"

/// @brief Interface for channel
class OPENMS_API IChannelReactor
{
public:
	virtual ~IChannelReactor() = default;

	virtual void startup() = 0;

	virtual void shutdown() = 0;

	virtual bool running() const = 0;

	virtual bool connect() const = 0;

	virtual MSHnd<IChannelAddress> address() const = 0;

	virtual void write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) = 0;

	virtual MSFuture<bool> write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address, MSPromise<bool>&& promise) = 0;

	virtual void writeAndFlush(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) = 0;

	virtual MSFuture<bool> writeAndFlush(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address, MSPromise<bool>&& promise) = 0;
};