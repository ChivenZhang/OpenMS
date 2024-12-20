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
#include "Mailbox.h"
#include "MailContext.h"

MailBox::MailBox(MSRaw<IMailContext> context)
	:
	m_Context(context)
{
}

bool MailBox::send(IMail&& mail)
{
	if (m_Context == nullptr) return false;
	return m_Context->sendToMailbox(std::forward<IMail>(mail));
}

void MailBox::sign(IMail&& mail)
{
	MS_INFO("TODO:implement sign method");
}

bool MailBox::create(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory)
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
