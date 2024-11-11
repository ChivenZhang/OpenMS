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
#include "IChannelEvent.h"

/// @brief Interface for channel
class OPENMS_API IChannelWorker
{
public:
	virtual ~IChannelWorker() = default;

	virtual void startup() = 0;

	virtual void shutdown() = 0;

	virtual bool running() const = 0;

	virtual void enqueue(TRef<IChannelEvent> event) = 0;
};