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
	void enqueue(MSRef<IMailBox> mailBox);
	size_t overload() const;
	void balance(MSDeque<MSRef<IMailBox>>& output);

protected:
	MSThread m_MailThread;
	MSRaw<MailHub> m_Context;
	MSMutex m_TaskLock;
	MSMutexUnlock m_TaskUnlock;
	MSDeque<MSRef<IMailBox>> m_TaskQueue;
	MSDeque<MSRef<IMailBox>> m_TempQueue;
};