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
#include "Reactor/IChannelAddress.h"

class OPENMS_API IEndpoint
{
public:
	virtual ~IEndpoint() = default;
	virtual void startup() = 0;
	virtual void shutdown() = 0;
	virtual bool running() const = 0;
	virtual bool connect() const = 0;
	virtual MSHnd<IChannelAddress> address() const = 0;
};