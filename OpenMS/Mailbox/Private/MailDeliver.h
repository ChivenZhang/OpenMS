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
#include "MailBox.h"
class MailContext;

/// @brief Implement for mail deliver
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