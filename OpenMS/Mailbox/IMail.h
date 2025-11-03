#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "MS.h"

/// @brief Interface for mail
struct IMail
{
	uint32_t From, To, Date;
	MSStringView Body;
};

/// @brief Interface for mail
struct IMailView
{
	uint32_t From, To, Date;
	char Body[0];
};