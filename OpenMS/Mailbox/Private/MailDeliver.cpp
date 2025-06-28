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
#include "MailDeliver.h"
#include "MailContext.h"
#include <coroutine>
#include <cpptrace/cpptrace.hpp>

MailDeliver::MailDeliver(MSRaw<MailContext> context)
	: m_Context(context)
{
	m_MailThread = MSThread([=]()
	{
		while (m_Context->m_Running)
		{
			MSUniqueLock lock(m_MailLock);
			MSHnd<IMailBox> element;
			m_Context->m_MailUnlock.wait(lock, [&]() { return m_Context->m_Running == false || m_Context->dequeueMailbox(element); });
			if (m_Context->m_Running == false) break;

			if (auto mailbox = MSCast<MailBox>(element.lock()))
			{
				MSMutexLock lock2(mailbox->m_MailLock);
				if (mailbox->m_MailQueue.size())
				{
					auto& mail = mailbox->m_MailQueue.front();
					if (mail.Handle.good() == false) mail.Handle = std::move(mailbox->read(std::move(mail.Mail)));
					if (mail.Handle.good() && mail.Handle.done() == false)
					{
						try
						{
							mail.Handle.resume();
						}
						catch (MSError ex)
						{
							mailbox->error(std::move(ex));
						}
						catch (...)
						{
							mailbox->error(cpptrace::logic_error("unknown exception"));
						}
					}
					if (mail.Handle.good() && mail.Handle.done()) mailbox->m_MailQueue.pop();
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
	if (m_MailThread.joinable()) m_MailThread.join();
	m_Context = nullptr;
}
