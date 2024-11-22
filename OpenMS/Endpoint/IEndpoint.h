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
#include "OpenMS/Reactor/IChannelAddress.h"

class OPENMS_API IEndpoint
{
public:
	virtual ~IEndpoint() = default;
	virtual void startup() = 0;
	virtual void shutdown() = 0;
	virtual bool running() const = 0;
	virtual bool connect() const = 0;
	virtual THnd<IChannelAddress> address() const = 0;
};