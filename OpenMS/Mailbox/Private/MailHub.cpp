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
#include "Mail.h"

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

bool MailHub::create(MSString address, MSRef<IMailBox> value)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.emplace(MSHash(address), nullptr);
	if (result.second == false) return false;
	auto mailbox = MSCast<MailBox>(value);
	if (mailbox == nullptr) return false;
	result.first->second = mailbox;
	mailbox->m_Context = this;
	mailbox->m_HashName = MSHash(address);
	mailbox->m_TextName = address;
	return true;
}

bool MailHub::cancel(MSString address)
{
	MSMutexLock lock(m_MailboxLock);
	return m_MailboxMap.erase(MSHash(address));
}

bool MailHub::exist(MSString address)
{
	MSMutexLock lock(m_MailboxLock);
	auto result = m_MailboxMap.find(MSHash(address));
	return result != m_MailboxMap.end();
}

uint32_t MailHub::send(IMail mail)
{
	MSRef<MailBox> toMailbox;
	if (mail.Date == 0) mail.Date = m_Session++;
	{
		MSMutexLock lock(m_MailboxLock);
		auto toName = (mail.Type & OPENMS_MAIL_TYPE_FORWARD) ? mail.Copy : mail.To;
		auto result = m_MailboxMap.find(toName);
		if (result == m_MailboxMap.end() || result->second == nullptr)
		{
			if (m_RemoteCall && m_RemoteCall(mail)) return mail.Date;
			return 0;
		}
		toMailbox = result->second;
	}
	{
		MSMutexLock lock(toMailbox->m_MailLock);
		auto idle = toMailbox->m_MailQueue.empty();
		MSString mailData(sizeof(MailView) + mail.Body.size(), 0);
		auto& mailView = *(MailView*)mailData.data();
		mailView.From = mail.From;
		mailView.To = mail.To;
		mailView.Copy = mail.Copy;
		mailView.Date = mail.Date;
		mailView.Type = mail.Type;
		if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());

		IMail newMail = {};
		newMail.From = mailView.From;
		newMail.To = mailView.To;
		newMail.Copy = mailView.Copy;
		newMail.Date = mailView.Date;
		newMail.Type = mailView.Type;
		newMail.Body = MSStringView(mailView.Body, mail.Body.size());
		toMailbox->m_MailQueue.push({ .Mail = std::move(mailData), .Task = toMailbox->read(newMail), });
		if (idle) enqueue(toMailbox);
		return mail.Date;
	}
}

bool MailHub::send(MSLambda<bool(IMail mail)> func)
{
	if (m_RemoteCall) return false;
	m_RemoteCall = func;
	return true;
}

void MailHub::list(MSList<uint32_t>& result)
{
	MSMutexLock lock(m_MailboxLock);
	for (auto& mailbox : m_MailboxMap)
	{
		result.push_back(mailbox.second->hash());
	}
}

bool MailHub::enqueue(MSHnd<IMailBox> mailbox)
{
	{
		MSMutexLock lock(m_MailLock);
		m_MailboxQueue.push(mailbox.lock());
	}
	m_MailUnlock.notify_all();
	return true;
}

bool MailHub::dequeue(MSHnd<IMailBox>& mailbox)
{
	MSMutexLock lock(m_MailLock);
	if (m_MailboxQueue.empty()) return false;
	mailbox = m_MailboxQueue.front();
	m_MailboxQueue.pop();
	return true;
}
