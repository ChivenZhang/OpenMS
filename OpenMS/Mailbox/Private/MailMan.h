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
class MailHub;

/// @brief Implement for mail man
class MailMan
{
public:
	explicit MailMan(MSRaw<MailHub> context);
	~MailMan();

protected:
	MSThread m_MailThread;
	MSRaw<MailHub> m_Context;
};