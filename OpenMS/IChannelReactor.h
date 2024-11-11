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
	struct callback_t
	{
		TLambda<void(TRef<IChannel>)> Connected;
		TLambda<void(TRef<IChannel>)> Disconnect;
	};

public:
	virtual ~IChannelReactor() = default;

	virtual void startup() = 0;

	virtual void shutdown() = 0;
};