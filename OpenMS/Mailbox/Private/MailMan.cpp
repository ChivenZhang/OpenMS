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
		MS_INFO("mail worker started");
		while (m_Context->m_Running)
		{
			// Pop mailbox from task queue

			MSRef<MailBox> mailbox;
			{
				MSUniqueLock mailboxLock(m_TaskLock);
				while (m_Context->m_Running && m_TaskQueue.empty())
				{
					// m_Context->balance(m_TaskQueue);
					// if (!m_TaskQueue.empty()) MS_INFO("steal %u", (uint32_t)m_TaskQueue.size());
					// if (!m_TaskQueue.empty()) break;
					// m_TaskCount += m_TaskQueue.size();
					m_TaskUnlock.wait_for(mailboxLock, std::chrono::seconds(1));
				}
				if (m_Context->m_Running == false) break;
				mailbox = MSCast<MailBox>(m_TaskQueue.front());
				m_TaskQueue.pop_front();
				m_TaskCount -= 1;
				if (mailbox == nullptr) continue;
			}

			// Pop mail from mailbox

			MailBox::mail_t mail;
			{
				MSMutexLock mailLock(mailbox->m_MailLock);
				if (mailbox->m_MailQueue.empty()) continue;
				mail = std::move(mailbox->m_MailQueue.front());
				mailbox->m_MailQueue.pop_front();

				// MS_INFO("pop %u=>%u via %u #%u", mail.Mail.From, mail.Mail.To, mail.Mail.Copy, mail.Mail.Date);
			}

			// Resume mail task

			if (mail.Task && mail.Task.done() == false)
			{
				switch (mail.Task.state())
				{
				case MSAsyncState::NONE: mail.Task.resume(); break;
				case MSAsyncState::YIELD: mail.Task.resume(); break;
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
					MS_DEBUG("done %u=>%u via %u #%u @%u", mail.Mail.From, mail.Mail.To, mail.Mail.Copy, mail.Mail.Date, mail.Mail.Type);
				}
				else
				{
					// Re-enqueue pending mail task

					MSMutexLock mailLock(mailbox->m_MailLock);
					switch (mail.Task.state())
					{
					case MSAsyncState::YIELD: mailbox->m_MailQueue.push_front(std::move(mail)); break;
					default: mailbox->m_MailQueue.push_back(std::move(mail)); break;
					}
				}
			}

			MSMutexLock mailLock(mailbox->m_MailLock);
			if (!mailbox->m_MailQueue.empty())
			{
				MSMutexLock mailboxLock(m_TaskLock);
				m_TaskQueue.push_back(mailbox);
				m_TaskCount += 1;
			}
		}
		MS_INFO("mail worker stopped");
	});
}

MailMan::~MailMan()
{
	m_TaskUnlock.notify_one();
	if (m_MailThread.joinable()) m_MailThread.join();
	m_Context = nullptr;
}

void MailMan::enqueue(MSRef<IMailBox> mailBox)
{
	{
		MSMutexLock lock(m_TaskLock);
		m_TaskQueue.push_back(mailBox);
		m_TaskCount += 1;
	}
	m_TaskUnlock.notify_one();
}

size_t MailMan::overload() const
{
	return m_TaskCount;
}

void MailMan::balance(MSDeque<MSRef<IMailBox>>& result)
{
	if (m_TaskCount.load())
	{
		MSMutexLock lock(m_TaskLock);
		size_t half = m_TaskQueue.size() >> 1;
		for (size_t i=0; i<half; ++i)
		{
			result.push_back(m_TaskQueue.back());
			m_TaskQueue.pop_back();
			m_TaskCount -= 1;
		}
	}
}
