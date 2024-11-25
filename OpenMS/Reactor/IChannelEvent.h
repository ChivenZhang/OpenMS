#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "MS.h"
class IChannel;

/// @brief Interface for channel
class OPENMS_API IChannelEvent
{
public:
	static TRef<IChannelEvent> New(TString&& message);
public:
	TString Message;
	THnd<IChannel> Channel;
	TRaw<TPromise<bool>> Promise = nullptr;
};