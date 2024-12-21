/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/20 17:04:01.
*
* =================================================*/
#include "MailDeliver.h"
#include "MailContext.h"
#include <coroutine>

MailDeliver::MailDeliver(MSRaw<MailContext> context)
	: m_Context(context)
{
	m_MailThread = MSThread([=]()
	{
		while (m_Context->m_Running)
		{
			MSUniqueLock lock(m_MailLock);
			MSHnd<IMailBox> element;
			m_Context->m_MailUnlock.wait(lock, [&](){ return m_Context->m_Running == false || m_Context->dequeueMailbox(element); });
			if (m_Context->m_Running == false) break;

			if (auto mailbox = MSCast<MailBox>(element.lock()))
			{
				MSMutexLock lock2(mailbox->m_MailLock);
				if (mailbox->m_MailQueue.size())
				{
					auto& mail = mailbox->m_MailQueue.front();
					if (mail.Handle.valid()) mail.Handle.resume();
					else mail.Handle = std::move(mailbox->sign(std::move(mail.Mail)));
					if (mail.Handle.done()) mailbox->m_MailQueue.pop();
				}
				if (mailbox->m_MailQueue.size())
				{
					m_Context->enqueueMailbox(mailbox);
				}
			}
		}
	});
}

MailDeliver::~MailDeliver()
{
	m_Context->m_MailUnlock.notify_one();
	if (m_MailThread.joinable()) m_MailThread.join();
	m_Context = nullptr;
}
