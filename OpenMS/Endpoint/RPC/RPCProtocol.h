#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "MS.h"

struct RPCRequestBase
{
	uint32_t Length;
	uint32_t Session;
};

struct RPCRequestView : RPCRequestBase
{
	uint32_t Method;
	char Buffer[0];
};

struct RPCResponseView : RPCRequestBase
{
	char Buffer[0];
};