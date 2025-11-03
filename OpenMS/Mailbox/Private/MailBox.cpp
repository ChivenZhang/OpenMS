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

MailBox::MailBox(MSRaw<IMailHub> context)
	:
	m_Context(context),
	m_HashName(0)
{
}

MSString MailBox::name() const
{
	return m_Address;
}

uint32_t MailBox::send(IMail mail)
{
	if (m_Context == nullptr) return 0;
	mail.From = m_HashName;
	return m_Context->sendToMailbox(mail);
}

bool MailBox::create(MSString address, MSLambda<MSRef<IMailBox>()> factory)
{
	if (m_Context == nullptr) return false;
	return m_Context->createMailbox(address, factory);
}

bool MailBox::cancel(MSString address)
{
	if (m_Context == nullptr) return false;
	return m_Context->cancelMailbox(address);
}

bool MailBox::exist(MSString address) const
{
	if (m_Context == nullptr) return false;
	return m_Context->existMailbox(address);
}

void MailBox::error(MSError&& info)
{
	MS_INFO("%s", info.what());
}

IMailTask MailBox::read(IMail mail)
{
	MS_INFO("TODO:implement read method");
	co_return;
}
