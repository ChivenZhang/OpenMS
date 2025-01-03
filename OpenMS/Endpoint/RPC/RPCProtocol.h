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
	uint32_t ID;
	MSString Name;
	MSString Args;
	OPENMS_TYPE(RPCRequest, ID, Name, Args)
};

struct RPCResponse
{
	uint32_t ID;
	MSString Args;
	OPENMS_TYPE(RPCResponse, ID, Args)
};

