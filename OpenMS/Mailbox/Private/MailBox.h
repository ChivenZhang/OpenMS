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
	MailBox() = default;
	~MailBox() override;
	MSString name() const final;
	uint32_t hash() const final;
	uint32_t send(IMail mail) final;
	using IMailBox::create;
	bool create(MSString address, MSRef<IMailBox> value) final;
	bool cancel(MSString address) final;
	bool exist(MSString address) const final;

protected:
	virtual void error(MSError&& info);
	virtual IMailTask read(IMail mail) = 0;

private:
	friend class MailHub;
	friend class MailMan;
	uint32_t m_HashName = 0;
	MSString m_TextName;
	MSRaw<IMailHub> m_Context = nullptr;
	MSMutex m_MailLock;
	MSAtomic<uint32_t> m_Session;
	struct mail_t { MSList<uint8_t> Data; IMail Mail; IMailTask Task;};
	MSDeque<mail_t> m_MailQueue;
};