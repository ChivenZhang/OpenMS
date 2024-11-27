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
#include "../IChannelEvent.h"

TRef<IChannelEvent> IChannelEvent::New(TCString message)
{
	auto result = TNew<IChannelEvent>();
	result->Message = message;
	return result;
}

TRef<IChannelEvent> IChannelEvent::New(TString&& message)
{
	auto result = TNew<IChannelEvent>();
	result->Message = std::move(message);
	return result;
}

TRef<IChannelEvent> IChannelEvent::New(TStringView message)
{
	auto result = TNew<IChannelEvent>();
	result->Message = message;
	return result;
}

TRef<IChannelEvent> IChannelEvent::New(TString const& message)
{
	auto result = TNew<IChannelEvent>();
	result->Message = message;
	return result;
}
