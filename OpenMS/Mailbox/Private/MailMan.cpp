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
			// Get mailbox from task queue

			MSRef<IMailBox> element;
			{
				MSUniqueLock mailboxLock(m_Context->m_MailTaskLock);
				m_Context->m_MailTaskUnlock.wait(mailboxLock, [this]()
				{
					return m_Context->m_Running == false || m_Context->m_MailTaskQueue.empty() == false;
				});
				if (m_Context->m_Running == false) break;
				element = m_Context->m_MailTaskQueue.front();
				m_Context->m_MailTaskQueue.pop();
			}

			// Process mailbox mail

			if (auto mailbox = MSCast<MailBox>(element))
			{
				// Dequeue mail from mailbox

				MailBox::mail_t mail;
				{
					MSMutexLock mailLock(mailbox->m_MailLock);
					if (mailbox->m_MailQueue.empty()) continue;
					mail = std::move(mailbox->m_MailQueue.front());
					mailbox->m_MailQueue.pop_front();

					MS_INFO("pop %u=>%u via %u #%u", mail.Mail.From, mail.Mail.To, mail.Mail.Copy, mail.Mail.Date);
				}

				// Resume mail task

				if (mail.Task && mail.Task.done() == false)
				{
					switch (mail.Task.state())
					{
					case MSAsyncState::NONE:
						{
							mail.Task.resume();
						} break;
					case MSAsyncState::YIELD:
						{
							mail.Task.resume();

							MSMutexLock mailLock(mailbox->m_MailLock);
							mailbox->m_MailQueue.push_front(std::move(mail));
						} break;
					default: break;
					}
				}

				if (mail.Task)
				{
					if (mail.Task.done())
					{
						// Handle completed mail task

						try
						{
							mail.Task.value();
						}
						catch(MSError& ex)
						{
							MSPrintError(ex);
						}
						MS_INFO("done %u=>%u via %u #%u", mail.Mail.From, mail.Mail.To, mail.Mail.Copy, mail.Mail.Date);
					}
					else
					{
						// Re-enqueue pending mail task

						MSMutexLock mailLock(mailbox->m_MailLock);
						mailbox->m_MailQueue.push_back(std::move(mail));
					}
				}

				// Re-enqueue mailbox if it still has mails

				auto isEmpty = false;
				{
					MSMutexLock mailLock(mailbox->m_MailLock);
					isEmpty = mailbox->m_MailQueue.empty();
				}
				if (isEmpty == false)
				{
					MSMutexLock mailboxLock(m_Context->m_MailTaskLock);
					m_Context->m_MailTaskQueue.push(mailbox);
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
