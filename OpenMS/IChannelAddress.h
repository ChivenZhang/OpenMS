#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "MS.h"

/// @brief Interface for channel address
class OPENMS_API IChannelAddress
{
public:
	virtual TString getAddress() const = 0;

	virtual TString getHostName() const = 0;
};

/// @brief Interface for channel socket address
class OPENMS_API IChannelSocketAddress : public IChannelAddress
{
public:
	virtual uint16_t getPort() const = 0;
};