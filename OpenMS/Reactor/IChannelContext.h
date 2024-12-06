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
#include "IChannelEvent.h"

/// @brief Interface for channel context
class OPENMS_API IChannelContext
{
public:
	virtual ~IChannelContext() = default;

	virtual void close() = 0;

	virtual MSFuture<bool> close(MSPromise<bool>& promise) = 0;

	virtual void write(MSRef<IChannelEvent> event) = 0;

	virtual MSFuture<bool> write(MSRef<IChannelEvent> event, MSPromise<bool>& promise) = 0;

	virtual void writeAndFlush(MSRef<IChannelEvent> event) = 0;

	virtual MSFuture<bool> writeAndFlush(MSRef<IChannelEvent> event, MSPromise<bool>& promise) = 0;
};