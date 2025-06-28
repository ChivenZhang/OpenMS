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
	static MSRef<IChannelEvent> New(MSCString message);
	static MSRef<IChannelEvent> New(MSString&& message);
	static MSRef<IChannelEvent> New(MSStringView message);
	static MSRef<IChannelEvent> New(MSString const& message);

public:
	MSString Message;
	MSHnd<IChannel> Channel;
	MSRaw<MSPromise<bool>> Promise = nullptr;
};