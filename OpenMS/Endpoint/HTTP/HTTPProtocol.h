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

struct HTTPRequest
{
	MSString Url;
	MSString Body;
	MSStringMap<MSString> Params;
	MSStringMap<MSString> Header;
};

struct HTTPResponse
{
	uint32_t Code;
	MSString Body;
	MSStringMap<MSString> Header;
};