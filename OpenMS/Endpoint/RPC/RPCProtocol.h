#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/20 14:07:21.
*
* =================================================*/
#include "MS.h"

struct RPCRequest
{
	uint32_t indx;
	MSString name;
	MSString args;
	OPENMS_TYPE(RPCRequest, indx, name, args)
};

struct RPCResponse
{
	uint32_t indx;
	MSString args;
	OPENMS_TYPE(RPCResponse, indx, args)
};

