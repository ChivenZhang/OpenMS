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
#define OPENMS_MAIL_TYPE_REQUEST 0x00000001
#define OPENMS_MAIL_TYPE_RESPONSE 0x00000002
#define OPENMS_MAIL_TYPE_FORWARD 0x00000004
#define OPENMS_MAIL_TYPE_CLIENT 0x00000008

/// @brief Interface for mail
struct IMail
{
	// 源服务
	uint32_t From;
	// 目标服务
	uint32_t To;
	// 抄送服务
	uint32_t Copy;
	// 时间戳
	uint32_t Date;
	// 消息类型（0,1表示内网请求，2表示内网响应，3表示外网请求，4表示外网响应）
	uint32_t Type;
	// 消息体
	MSStringView Body;
};