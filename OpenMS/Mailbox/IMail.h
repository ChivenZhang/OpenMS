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
	uint32_t From, To;	// 源服务，目标服务
	uint32_t Date, Addr;// 时间戳，目标地址 <保留域>
	MSStringView Body;	// 消息体
};