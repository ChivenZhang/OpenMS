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
#include "../Private/Property.h"
#include "Mailbox/Private/MailContext.h"

/// @brief
class ClusterConfig
	:
	RESOURCE2(Property, IProperty),
	RESOURCE2(MailContext, IMailContext)
{
};