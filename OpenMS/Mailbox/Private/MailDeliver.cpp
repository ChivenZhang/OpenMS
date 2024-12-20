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

MailDeliver::MailDeliver(MSRaw<MailContext> context)
	: m_Context(context)
{
	m_MailThread = MSThread([=]()
	{
		while (m_Context->m_Running)
		{
			MSUniqueLock lock(m_MailLock);
			m_Context->m_MailUnlock.wait(lock);
			if (m_Context->m_Running == false) break;
			MSHnd<IMailBox> element;
			if (m_Context->dequeueMailbox(element))
			{
				if (auto mailbox = MSCast<MailBox>(element.lock()))
				{
					MSMutexLock lock2(mailbox->m_MailLock);
					if (mailbox->m_MailQueue.size())
					{
						auto mail = mailbox->m_MailQueue.front();
						mailbox->m_MailQueue.pop();
						mailbox->sign(std::move(mail));
					}
					if (mailbox->m_MailQueue.size())
					{
						m_Context->enqueueMailbox(mailbox);
					}
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
