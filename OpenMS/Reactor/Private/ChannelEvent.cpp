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
#include "../IChannelEvent.h"

MSRef<IChannelEvent> IChannelEvent::New(MSStringView const& message, MSHnd<IChannel> const& channel)
{
	auto result = MSNew<IChannelEvent>();
	result->Message = message;
	result->Channel = channel;
	return result;
}
