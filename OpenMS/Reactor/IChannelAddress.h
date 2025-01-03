#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "MS.h"

/// @brief Interface for channel address
class OPENMS_API IChannelAddress
{
public:
	virtual MSString getAddress() const = 0;

	virtual MSString getHostName() const = 0;

	virtual uint32_t getHashName() const = 0;

	virtual MSString getString() const = 0;
};

/// @brief Interface for socket address
class OPENMS_API ISocketAddress : public IChannelAddress
{
public:
	virtual uint16_t getPort() const = 0;
};