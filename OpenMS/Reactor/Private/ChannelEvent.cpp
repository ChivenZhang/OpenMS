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

TRef<IChannelEvent> IChannelEvent::New(TString&& message)
{
	auto result = TNew<IChannelEvent>();
	result->Message = message;
	return result;
}