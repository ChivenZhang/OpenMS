#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/20 17:00:55.
*
* =================================================*/
#include "../IMailContext.h"
#include "MailBox.h"
#include "MailDeliver.h"

class MailContext : public IMailContext
{
public:
	explicit MailContext(uint32_t overload = MSThread::hardware_concurrency());
	~MailContext() override;
	using IMailContext::createMailbox;
	bool createMailbox(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory) override;
	bool cancelMailbox(MSString address) override;
	bool existMailbox(MSString address) override;
	bool sendToMailbox(IMail&& mail) override;
	void listMailbox(MSStringList& result) override;

private:
	friend class MailDeliver;
	bool enqueueMailbox(MSHnd<IMailBox> mailbox);
	bool dequeueMailbox(MSHnd<IMailBox>& mailbox);

protected:
	MSMutex m_MailLock;
	MSMutexUnlock m_MailUnlock;
	MSMutex m_MailboxLock;
	MSMutexUnlock m_MailboxUnlock;
	MSAtomic<bool> m_Running;
	MSList<MSRef<MailBox>> m_Mailboxes;
	MSList<MSRef<MailDeliver>> m_Delivers;
	MSMap<MSString, MSRef<MailBox>> m_MailboxMap;
	MSQueue<MSRef<IMailBox>> m_MailboxQueue;
};