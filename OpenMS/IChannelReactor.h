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
#include "IChannel.h"

/// @brief Interface for channel
class OPENMS_API IChannelReactor
{
public:
	virtual ~IChannelReactor() = default;

	virtual void startup() = 0;

	virtual void shutdown() = 0;

	virtual void write(TRef<IChannelEvent> event, TRef<IChannelAddress> address) = 0;

	virtual TFuture<bool> write(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise) = 0;

	virtual void writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address) = 0;

	virtual TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise) = 0;
};