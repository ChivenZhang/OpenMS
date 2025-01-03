#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 19:21:53.
*
* =================================================*/
#include "../Private/Property.h"
#include "Mailbox/Private/MailContext.h"

/// @brief
class ClusterConfig
	:
	RESOURCE2(Property, IProperty),
	RESOURCE2(MailContext, IMailContext)
{
};