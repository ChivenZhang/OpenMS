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
				m_TaskLock.lock();
				if (m_TaskQueue.empty())
				{
					m_TaskLock.unlock();
					m_Context->balance(m_TempQueue);
					if (m_TempQueue.empty())
					{
						MSUniqueLock mailboxLock(m_TaskLock);
						m_TaskUnlock.wait_for(mailboxLock, std::chrono::milliseconds(500));
						continue;
					}
					MS_INFO("steal %u", (uint32_t)m_TempQueue.size());
					MSMutexLock lock(m_TaskLock);
					while (!m_TempQueue.empty())
					{
						m_TaskQueue.push_back(m_TempQueue.front());
						m_TempQueue.pop_front();
					}
					mailbox = MSCast<MailBox>(m_TaskQueue.front());
					m_TaskQueue.pop_front();
				}
				else
				{
					mailbox = MSCast<MailBox>(m_TaskQueue.front());
					m_TaskQueue.pop_front();
					m_TaskLock.unlock();
				}

				if (mailbox == nullptr) continue;
			}

			// Pop mail from mailbox

			MailBox::mail_t mail;
			{
				MSMutexLock mailLock(mailbox->m_MailLock);
				if (mailbox->m_MailQueue.empty()) continue;
				mail = std::move(mailbox->m_MailQueue.front());
				mailbox->m_MailQueue.pop_front();
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
			}
		}
		MS_INFO("mail worker stopped");
	});
}

MailMan::~MailMan()
{
	m_TaskUnlock.notify_all();
	if (m_MailThread.joinable()) m_MailThread.join();
	m_Context = nullptr;
}

void MailMan::enqueue(MSRef<IMailBox> mailBox)
{
	{
		MSMutexLock lock(m_TaskLock);
		m_TaskQueue.push_back(mailBox);
	}
	m_TaskUnlock.notify_all();
}

void MailMan::balance(MSDeque<MSRef<IMailBox>>& result)
{
	MSMutexLock lock(m_TaskLock);
	if (m_TaskQueue.empty()) return;
	auto middle = m_TaskQueue.begin() + (long long)m_TaskQueue.size();
	result.insert(result.end(), std::make_move_iterator(middle), std::make_move_iterator(m_TaskQueue.end()));
	m_TaskQueue.erase(middle, m_TaskQueue.end());
}
