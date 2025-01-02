#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2025/01/01 09:02:13.
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