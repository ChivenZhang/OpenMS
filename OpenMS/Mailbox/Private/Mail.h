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

/// @brief view for mail
struct MailView
{
	uint32_t From, To;	// 源服务，目标服务
	uint32_t Date, Type;// 时间戳，消息类型
	char Body[0];
};