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

// struct RPCRequest
// {
// 	uint32_t ID;
// 	uint32_t Name;
// 	MSString Args;
// };
//
// struct RPCResponse
// {
// 	uint32_t ID;
// 	MSString Args;
// };

struct RPCRequestView
{
	uint32_t ID;
	uint32_t Name;
	uint32_t Length;
	char Buffer[0];
};

struct RPCResponseView
{
	uint32_t ID;
	uint32_t Length;
	char Buffer[0];
};

