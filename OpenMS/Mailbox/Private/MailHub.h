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
#include "../IMailHub.h"
#include "MailBox.h"
#include "MailMan.h"

class MailHub : public IMailHub
{
public:
	explicit MailHub(uint32_t overload = MSThread::hardware_concurrency());
	~MailHub() override;
	using IMailHub::createMailbox;
	bool createMailbox(MSString address, MSLambda<MSRef<IMailBox>()> factory) override;
	bool cancelMailbox(MSString address) override;
	bool existMailbox(MSString address) override;
	uint32_t sendToMailbox(IMail mail) override;
	bool sendToMailbox(MSLambda<bool(IMail mail)> func) override;
	void listMailbox(MSStringList& result) override;

private:
	friend class MailMan;
	bool enqueueMailbox(MSHnd<IMailBox> mailbox);
	bool dequeueMailbox(MSHnd<IMailBox>& mailbox);

protected:
	MSMutex m_MailLock;
	MSMutexUnlock m_MailUnlock;
	MSMutex m_MailboxLock;
	MSMutexUnlock m_MailboxUnlock;
	MSAtomic<bool> m_Running;
	MSLambda<bool(IMail mail)> m_RemoteCall;
	MSAtomic<uint32_t> m_Session;
	MSList<MSRef<MailBox>> m_Mailboxes;
	MSList<MSRef<MailMan>> m_Delivers;
	MSMap<uint32_t, MSRef<MailBox>> m_MailboxMap;
	MSQueue<MSRef<IMailBox>> m_MailboxQueue;
};