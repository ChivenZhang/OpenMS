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
	using IMailHub::create;
	bool create(MSString address, MSRef<IMailBox> value) override;
	bool cancel(MSString address) override;
	bool exist(MSString address) override;
	uint32_t send(IMail mail) override;
	void list(MSList<uint32_t>& result) override;
	bool failed(MSLambda<bool(IMail mail)> callback) override;
	bool change(MSLambda<void(MSString address)> callback) override;

protected:
	friend class MailMan;

	MSAtomic<bool> m_Running;
	MSLambda<bool(IMail mail)> m_OnFailed;
	MSLambda<void(MSString address)> m_OnChange;

	MSMutex m_MailboxLock;
	MSAtomic<uint32_t> m_MailSession;
	MSList<MSRef<MailMan>> m_MailDelivers;
	MSMap<uint32_t, MSRef<MailBox>> m_MailboxMap;

	MSMutex m_MailTaskLock;
	MSMutexUnlock m_MailTaskUnlock;
	MSQueue<MSRef<IMailBox>> m_MailTaskQueue;
};