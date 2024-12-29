#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 16:02:46.
*
* =================================================*/
#include "../Private/Property.h"
#include "Mailbox/Private/MailContext.h"

/// @brief 
class MasterConfig
	:
	RESOURCE2(Property, IProperty),
	RESOURCE2(MailContext, IMailContext)
{
};