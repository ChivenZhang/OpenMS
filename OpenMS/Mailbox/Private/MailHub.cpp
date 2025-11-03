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
#include "MailHub.h"

MailHub::MailHub(uint32_t overload)
	:
	m_Delivers(std::max(1U, overload))
{
	m_Session = 1;
	m_Running = true;
	for (auto& deliver : m_Delivers)
	{
		deliver = MSNew<MailMan>(this);
	}
}

MailHub::~MailHub()
{
	m_Running = false;
	m_MailUnlock.notify_all();
	m_Delivers.clear();
}

bool MailHub::create(MSString address, MSLambda<MSRef<IMailBox>()> factory)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.emplace(MSHash(address), nullptr);
	if (result.second == false) return false;
	auto mailbox = MSCast<MailBox>(factory());
	if (mailbox == nullptr) return false;
	mailbox->m_Address = address;
	mailbox->m_HashName = MSHash(address);
	m_Mailboxes.push_back(mailbox);
	result.first->second = m_Mailboxes.back();
	return true;
}

bool MailHub::cancel(MSString address)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.find(MSHash(address));
	if (result == m_MailboxMap.end()) return false;
	std::erase(m_Mailboxes, result->second);
	m_MailboxMap.erase(result);
	return true;
}

bool MailHub::exist(MSString address)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.find(MSHash(address));
	if (result == m_MailboxMap.end()) return false;
	return true;
}

uint32_t MailHub::send(IMail mail)
{
	MSRef<MailBox> toMailbox;
	mail.Date = m_Session++;
	{
		MSMutexLock lock(m_MailboxLock);
		auto result = m_MailboxMap.find(mail.To);
		if (result == m_MailboxMap.end() || result->second == nullptr)
		{
			if (m_RemoteCall == nullptr) return 0;
			if (m_RemoteCall(mail)) return mail.Date;
			return 0;
		}
		toMailbox = result->second;
	}
	{
		MSMutexLock lock(toMailbox->m_MailLock);
		auto idle = toMailbox->m_MailQueue.empty();
		MSString mailData(sizeof(IMailView) + mail.Body.size(), 0);
		auto& mailView = *(IMailView*)mailData.data();
		mailView.From = mail.From;
		mailView.To = mail.To;
		mailView.Date = mail.Date;
		if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
		toMailbox->m_MailQueue.push({ .Mail = std::move(mailData) });
		if (idle) enqueueMailbox(toMailbox);
		return mail.Date;
	}
}

bool MailHub::send(MSLambda<bool(IMail mail)> func)
{
	if (m_RemoteCall) return false;
	m_RemoteCall = func;
	return true;
}

void MailHub::list(MSStringList& result)
{
	MSMutexLock lock(m_MailboxLock);
	for (auto& mailbox : m_Mailboxes)
	{
		result.push_back(mailbox->m_Address);
	}
}

bool MailHub::enqueueMailbox(MSHnd<IMailBox> mailbox)
{
	{
		MSMutexLock lock(m_MailLock);
		m_MailboxQueue.push(mailbox.lock());
	}
	m_MailUnlock.notify_one();
	return true;
}

bool MailHub::dequeueMailbox(MSHnd<IMailBox>& mailbox)
{
	MSMutexLock lock(m_MailLock);
	if (m_MailboxQueue.empty()) return false;
	mailbox = m_MailboxQueue.front();
	m_MailboxQueue.pop();
	return true;
}
