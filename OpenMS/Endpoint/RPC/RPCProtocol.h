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

struct RPCProtocolParser
{
	template<class T>
	static bool toString(T const& input, MSString& output)
	{
		return MSTypeC(input, output);
	}
	template<class T>
	static bool fromString(MSString const& input, T& output)
	{
		return MSTypeC(input, output);
	}
};