#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/20 17:04:01.
*
* =================================================*/
#include "MailBox.h"
class MailContext;

/// @brief 
class MailDeliver
{
public:
	explicit MailDeliver(MSRaw<MailContext> context);
	~MailDeliver();

protected:
	MSMutex m_MailLock;
	MSThread m_MailThread;
	MSRaw<MailContext> m_Context;
};