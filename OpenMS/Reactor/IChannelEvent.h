#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "MS.h"
class IChannel;

/// @brief Interface for channel
class OPENMS_API IChannelEvent
{
public:
	static MSRef<IChannelEvent> New(MSStringView const& message, MSHnd<IChannel> const& channel = {});

public:
	MSString Message;
	MSHnd<IChannel> Channel;
	MSRaw<MSPromise<bool>> Promise = nullptr;
};