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
	m_MailDelivers(std::max(1U, overload))
{
	m_Running = true;
	m_MailSession = 1;
	for (auto& deliver : m_MailDelivers)
	{
		deliver = MSNew<MailMan>(this);
	}
}

MailHub::~MailHub()
{
	m_Running = false;
	m_MailTaskUnlock.notify_all();
	m_MailDelivers.clear();
}

bool MailHub::create(MSString address, MSRef<IMailBox> value)
{
	MSMutexLock lock(m_MailboxLock);
	auto mailbox = MSCast<MailBox>(value);
	if (mailbox == nullptr) return false;
	mailbox->m_TextName = address;
	mailbox->m_HashName = MSHash(address);
	MS_INFO("create mailbox %s, %u", address.c_str(), MSHash(address));
	auto result = m_MailboxMap.emplace(mailbox->m_HashName, nullptr);
	if (result.second == false)
	{
		MS_ERROR("repeated create %s, %u", mailbox->name().c_str(), mailbox->hash());
		return false;
	}
	result.first->second = mailbox;
	mailbox->m_Context = this;
	if (m_OnChange) m_OnChange(address);
	return true;
}

bool MailHub::cancel(MSString address)
{
	MSMutexLock lock(m_MailboxLock);
	MS_INFO("delete mailbox %s, %u", address.c_str(), MSHash(address));
	auto result = m_MailboxMap.erase(MSHash(address));
	if (result && m_OnChange) m_OnChange(address);
	return result;
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
	if (mail.Date == 0) mail.Date = m_MailSession++;
	{
		MSMutexLock mailboxLock(m_MailboxLock);
		auto toName = (mail.Type & OPENMS_MAIL_TYPE_FORWARD) ? mail.Copy : mail.To;
		auto result = m_MailboxMap.find(toName);
		if (result == m_MailboxMap.end() || result->second == nullptr)
		{
			if (m_OnFailed && m_OnFailed(mail)) return mail.Date;
			return 0;
		}
		toMailbox = result->second;
	}
	{
		MSMutexLock mailLock(toMailbox->m_MailLock);
		auto isIdle = toMailbox->m_MailQueue.empty();
		auto& newMail = toMailbox->m_MailQueue.emplace_back();
		newMail.Data.assign((uint8_t*)mail.Body.data(), (uint8_t*)mail.Body.data() + mail.Body.size());
		newMail.Mail.From = mail.From;
		newMail.Mail.To = mail.To;
		newMail.Mail.Copy = mail.Copy;
		newMail.Mail.Date = mail.Date;
		newMail.Mail.Type = mail.Type;
		newMail.Mail.Body = MSStringView((char*)newMail.Data.data(), newMail.Data.size());
		newMail.Task = toMailbox->read(newMail.Mail);
		MS_INFO("push %u=>%u via %u #%u", mail.From, mail.To, mail.Copy, mail.Date);

		if (isIdle)
		{
			MSMutexLock mailboxLock(m_MailTaskLock);
			m_MailTaskQueue.push(toMailbox);
			m_MailTaskUnlock.notify_one();
		}
	}
	return mail.Date;
}

void MailHub::list(MSList<uint32_t>& result)
{
	MSMutexLock lock(m_MailboxLock);
	for (auto& mailbox : m_MailboxMap)
	{
		result.push_back(mailbox.second->hash());
	}
}

bool MailHub::failed(MSLambda<bool(IMail mail)> callback)
{
	if (m_OnFailed) return false;
	m_OnFailed = callback;
	return true;
}

bool MailHub::change(MSLambda<void(MSString address)> callback)
{
	if (m_OnChange) return false;
	m_OnChange = callback;
	return true;
}