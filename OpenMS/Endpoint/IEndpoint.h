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
#include "MS.h"

class OPENMS_API IEndpoint
{
public:
	virtual ~IEndpoint() = default;
	virtual void startup() = 0;
	virtual void shutdown() = 0;
};