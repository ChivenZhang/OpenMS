#pragma once
#include "MS.h"
#include "Reactor/IChannel.h"
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

class IMessageSession
{
public:
	MSHnd<IChannel> Channel;
};