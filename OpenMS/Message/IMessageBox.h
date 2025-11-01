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
#include "Utility/Async.h"
#include "IMessage.h"
#include "IMessageSession.h"

class IMessageBox
{
public:
	virtual MSAsync<MSString> handle(MSRef<IMessageSession> session, IMessage const& message) = 0;
};
