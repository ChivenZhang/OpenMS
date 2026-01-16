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
			MSRef<IMailBox> element;
			{
				MSUniqueLock mailboxLock(m_Context->m_MailboxLock);
				m_Context->m_MailboxUnlock.wait(mailboxLock, [this]()
				{
					return m_Context->m_Running == false || m_Context->m_MailboxQueue.empty() == false;
				});
				if (m_Context->m_Running == false) break;
				element = m_Context->m_MailboxQueue.front();
				m_Context->m_MailboxQueue.pop();
			}

			if (auto mailbox = MSCast<MailBox>(element))
			{
				MailBox::mail_t mail;
				{
					MSMutexLock mailLock(mailbox->m_MailLock);
					if (mailbox->m_MailQueue.empty()) continue;
					mail = std::move(mailbox->m_MailQueue.front());
					mailbox->m_MailQueue.pop_front();
				}
				if (mail.Task && !mail.Task.done())
				{
					if (mail.Task.state() == MSAsyncState::NONE || mail.Task.state() == MSAsyncState::YIELD)
					{
						mail.Task.resume();
					}
				}
				try
				{
					if (mail.Task)
					{
						if (mail.Task.done()) mail.Task.value();
						else
						{
							MSMutexLock mailLock(mailbox->m_MailLock);
							mailbox->m_MailQueue.push_back(std::move(mail));
						}
					}
				}
				catch(MSError& ex)
				{
					MSPrintError(ex);
				}
				{
					auto isEmpty = false;
					{
						MSMutexLock mailLock(mailbox->m_MailLock);
						isEmpty = mailbox->m_MailQueue.empty();
					}
					if (!isEmpty)
					{
						MSMutexLock mailboxLock(m_Context->m_MailboxLock);
						m_Context->m_MailboxQueue.push(mailbox);
					}
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
