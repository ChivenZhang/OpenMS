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
#include "../IChannelEvent.h"

MSRef<IChannelEvent> IChannelEvent::New(MSCString message)
{
	auto result = MSNew<IChannelEvent>();
	result->Message = message;
	return result;
}

MSRef<IChannelEvent> IChannelEvent::New(MSString&& message)
{
	auto result = MSNew<IChannelEvent>();
	result->Message = std::move(message);
	return result;
}

MSRef<IChannelEvent> IChannelEvent::New(MSStringView message)
{
	auto result = MSNew<IChannelEvent>();
	result->Message = message;
	return result;
}

MSRef<IChannelEvent> IChannelEvent::New(MSString const& message)
{
	auto result = MSNew<IChannelEvent>();
	result->Message = message;
	return result;
}
