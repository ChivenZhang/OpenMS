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

/// @brief Interface for channel context
class OPENMS_API IChannelContext
{
public:
	virtual ~IChannelContext() = default;

	virtual void close() = 0;

	virtual TFuture<bool> close(TPromise<bool>&& promise) = 0;

	virtual void write(TRef<IChannelEvent> event) = 0;

	virtual TFuture<bool> write(TRef<IChannelEvent> event, TPromise<bool>&& promise) = 0;

	virtual void writeAndFlush(TRef<IChannelEvent> event) = 0;

	virtual TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TPromise<bool>&& promise) = 0;
};