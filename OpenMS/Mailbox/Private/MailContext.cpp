/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/20 17:00:55.
*
* =================================================*/
#include "MailContext.h"

MailContext::MailContext(uint32_t overload)
	:
	m_Delivers(std::max(1U, overload))
{
	m_Running = true;

	for (size_t i = 0; i < m_Delivers.size(); ++i)
	{
		m_Delivers[i] = MSNew<MailDeliver>(this);
	}
}

MailContext::~MailContext()
{
	m_Running = false;
	m_MailUnlock.notify_one();
	m_Delivers.clear();
}

bool MailContext::createMailbox(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.emplace(address, nullptr);
	if (result.second == false) return false;
	auto mailbox = MSCast<MailBox>(factory(this));
	if (mailbox == nullptr) return false;
	mailbox->m_Address = address;
	m_Mailboxes.push_back(mailbox);
	result.first->second = m_Mailboxes.back();
	return true;
}

bool MailContext::cancelMailbox(MSString address)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.find(address);
	if (result == m_MailboxMap.end()) return false;
	std::erase(m_Mailboxes, result->second);
	m_MailboxMap.erase(result);
	return true;
}

bool MailContext::existMailbox(MSString address)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.find(address);
	if (result == m_MailboxMap.end()) return false;
	return true;
}

bool MailContext::sendToMailbox(IMail&& mail)
{
	MSRef<MailBox> mailbox;
	{
		MSMutexLock lock(m_MailboxLock);
		auto result = m_MailboxMap.find(mail.To);
		if (result == m_MailboxMap.end() || result->second == nullptr)
		{
			if (m_RemoteCall == nullptr) return false;
			return m_RemoteCall(std::forward<IMail>(mail));
		}
		mailbox = result->second;
	}
	{
		MSMutexLock lock(mailbox->m_MailLock);
		if (mail.SID == 0) mail.SID = ++mailbox->m_Session;
		auto idle = mailbox->m_MailQueue.empty();
		mailbox->m_MailQueue.push({std::forward<IMail>(mail)});
		if (idle) enqueueMailbox(mailbox);
		return true;
	}
}

bool MailContext::sendToMailbox(MSLambda<bool(IMail&& mail)> func)
{
	if (m_RemoteCall) return false;
	m_RemoteCall = func;
	return true;
}

void MailContext::listMailbox(MSStringList& result)
{
	MSMutexLock lock(m_MailboxLock);
	for (auto mailbox : m_Mailboxes)
	{
		result.push_back(mailbox->m_Address);
	}
}

bool MailContext::enqueueMailbox(MSHnd<IMailBox> mailbox)
{
	{
		MSMutexLock lock(m_MailLock);
		m_MailboxQueue.push(mailbox.lock());
	}
	m_MailUnlock.notify_one();
	return true;
}

bool MailContext::dequeueMailbox(MSHnd<IMailBox>& mailbox)
{
	MSMutexLock lock(m_MailLock);
	if (m_MailboxQueue.empty()) return false;
	mailbox = m_MailboxQueue.front();
	m_MailboxQueue.pop();
	return true;
}
