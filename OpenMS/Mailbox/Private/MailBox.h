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
#include "../IMailBox.h"
class IChannel;
class MailHub;
class MailMan;

/// @brief 
class MailBox : public IMailBox
{
public:
	explicit MailBox(MSRaw<IMailHub> context);
	MSString name() const final;
	uint32_t send(IMail mail) final;
	using IMailBox::create;
	bool create(MSString address, MSLambda<MSRef<IMailBox>()> factory) final;
	bool cancel(MSString address) final;
	bool exist(MSString address) const final;

protected:
	void error(MSError&& info) override;
	IMailTask read(IMail mail) override;

private:
	friend class MailHub;
	friend class MailMan;
	MSString m_Address;
	uint32_t m_HashName;
	MSMutex m_MailLock;
	MSAtomic<uint32_t> m_Session;
	MSRaw<IMailHub> m_Context;
	struct mail_t { MSString Mail; IMailTask Handle; };
	MSQueue<mail_t> m_MailQueue;
};
