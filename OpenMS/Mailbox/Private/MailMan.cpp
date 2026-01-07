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
#include "MailMan.h"
#include "MailHub.h"
#include "Mail.h"
#include <coroutine>
#include <cpptrace/cpptrace.hpp>

MailMan::MailMan(MSRaw<MailHub> context)
	:
	m_Context(context)
{
	m_MailThread = MSThread([this]()
	{
		while (m_Context->m_Running)
		{
			MSHnd<IMailBox> element;
			{
				MSUniqueLock mailboxLock(m_Context->m_MailLock);
				while (m_Context->m_Running && m_Context->dequeue(element) == false)
				{
					m_Context->m_MailUnlock.wait(mailboxLock);
				}
				if (m_Context->m_Running == false) break;
			}

			if (auto mailbox = MSCast<MailBox>(element.lock()))
			{
				MSMutexLock mailLock(mailbox->m_MailLock);
				if (mailbox->m_MailQueue.empty() == false)
				{
					auto& mail = mailbox->m_MailQueue.front();
					if (bool(mail.Task) == true && mail.Task.done() == false)
					{
						if (mail.Task.state() == MSAsyncState::NONE || mail.Task.state() == MSAsyncState::YIELD)
						{
							mail.Task.resume();
						}
						else if(mail.Task.state() == MSAsyncState::AWAIT)
						{
							mailbox->m_MailQueue.push({ .Mail = std::move(mail.Mail), .Task = std::move(mail.Task), });
						}
					}
					if (bool(mail.Task) == false || mail.Task.done() == true)
					{
						try
						{
							if (mail.Task) mail.Task.value();
						}
						catch(MSError& ex)
						{
							MSPrintError(ex);
						}
						mailbox->m_MailQueue.pop();
					}
				}
				if (mailbox->m_MailQueue.empty() == false)
				{
					MSMutexLock mailboxLock(m_Context->m_MailLock);
					m_Context->enqueue(mailbox);
				}
			}
		}
	});
}

MailMan::~MailMan()
{
	if (m_MailThread.joinable()) m_MailThread.join();
	m_Context = nullptr;
}
