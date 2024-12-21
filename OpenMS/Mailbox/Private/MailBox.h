#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
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
	bool send(IMail&& mail) final override;
	IMailResult sign(IMail&& mail) override;
	using IMailBox::create;
	bool create(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory) final override;
	bool cancel(MSString address) final override;
	bool exist(MSString address) const final override;

private:
	friend class MailContext;
	friend class MailDeliver;
	MSMutex m_MailLock;
	struct mail_t { IMail Mail; IMailResult Handle; };
	MSQueue<mail_t> m_MailQueue;
	MSRaw<IMailContext> m_Context;
	MSAtomic<uint32_t> m_MailSession;
};
