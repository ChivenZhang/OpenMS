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
#include "MailHub.h"

MailBox::~MailBox()
{
	while (m_MailQueue.empty() == false)
	{
		auto& handle = m_MailQueue.front().Task;
		while (handle && handle.done() == false) std::this_thread::yield();
		m_MailQueue.pop();
	}
}

MSString MailBox::name() const
{
	return m_TextName;
}

uint32_t MailBox::hash() const
{
	return m_HashName;
}

uint32_t MailBox::send(IMail mail)
{
	if (m_Context == nullptr) return 0;
	return m_Context->send(mail);
}

bool MailBox::create(MSString address, MSRef<IMailBox> value)
{
	if (m_Context == nullptr) return false;
	return m_Context->create(address, value);
}

bool MailBox::cancel(MSString address)
{
	if (m_Context == nullptr) return false;
	return m_Context->cancel(address);
}

bool MailBox::exist(MSString address) const
{
	if (m_Context == nullptr) return false;
	return m_Context->exist(address);
}

void MailBox::error(MSError&& info)
{
	MS_ERROR("%s", info.what());
}