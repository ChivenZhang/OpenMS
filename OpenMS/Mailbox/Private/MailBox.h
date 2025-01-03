#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/20 17:07:32.
*
* =================================================*/
#include "../IMailBox.h"
class MailContext;
class MailDeliver;

/// @brief 
class MailBox : public IMailBox
{
public:
	explicit MailBox(MSRaw<IMailContext> context);
	bool send(IMail&& mail) final;
	using IMailBox::create;
	bool create(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory) final;
	bool cancel(MSString address) final;
	bool exist(MSString address) const final;

protected:
	void error(MSError&& info) override;
	IMailTask<void> read(IMail&& mail) override;

private:
	friend class MailContext;
	friend class MailDeliver;
	MSString m_Address;
	MSMutex m_MailLock;
	MSAtomic<uint32_t> m_Session;
	MSRaw<IMailContext> m_Context;
	struct mail_t { IMail Mail; IMailTask<void> Handle; };
	MSQueue<mail_t> m_MailQueue;
};
