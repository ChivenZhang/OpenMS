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
#include <random>

MailHub::MailHub(uint32_t overload)
	:
	m_MailWorkers(std::max(1U, overload))
{
	m_Running = true;
	for (auto& deliver : m_MailWorkers)
	{
		deliver = MSNew<MailMan>(this);
	}
}

MailHub::~MailHub()
{
	m_Running = false;
	m_MailWorkers.clear();
}

bool MailHub::create(MSString address, MSRef<IMailBox> value)
{
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
	}
	if (m_OnChange) m_OnChange(address);
	return true;
}

bool MailHub::cancel(MSString address)
{
	{
		MSMutexLock lock(m_MailboxLock);
		MS_INFO("delete mailbox %s, %u", address.c_str(), MSHash(address));
		auto result = m_MailboxMap.erase(MSHash(address));
		if (result == 0) return false;
	}
	if (m_OnChange) m_OnChange(address);
	return true;
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
	{
		m_MailboxLock.lock();
		auto toName = (mail.Type & OPENMS_MAIL_TYPE_FORWARD) ? mail.Copy : mail.To;
		auto result = m_MailboxMap.find(toName);
		if (result == m_MailboxMap.end() || result->second == nullptr)
		{
			m_MailboxLock.unlock();
			MS_INFO("send %u=>%u via %u #%u @%u", mail.From, mail.To, mail.Copy, mail.Date, mail.Type);
			if (m_OnFailed && m_OnFailed(mail)) return mail.Date;
			return 0;
		}
		else
		{
			toMailbox = result->second;
			m_MailboxLock.unlock();
		}
	}
	{
		MSMutexLock mailLock(toMailbox->m_MailLock);
		auto& newMail = toMailbox->m_MailQueue.emplace_back();
		newMail.Data.assign((uint8_t*)mail.Body.data(), (uint8_t*)mail.Body.data() + mail.Body.size());
		newMail.Mail.From = mail.From;
		newMail.Mail.To = mail.To;
		newMail.Mail.Copy = mail.Copy;
		newMail.Mail.Date = mail.Date;
		newMail.Mail.Type = mail.Type;
		newMail.Mail.Body = MSStringView((char*)newMail.Data.data(), newMail.Data.size());
		newMail.Task = toMailbox->read(newMail.Mail);
		MS_INFO("push %u=>%u via %u #%u @%u", mail.From, mail.To, mail.Copy, mail.Date, mail.Type);
		MS_INFO("size %u", (uint32_t)toMailbox->m_MailQueue.size());

		if (toMailbox->m_MailQueue.size() == 1)
		{
			MS_INFO("join %u=>%u via %u #%u @%u", mail.From, mail.To, mail.Copy, mail.Date, mail.Type);
			thread_local std::mt19937 gen(std::random_device{}());
			std::uniform_int_distribution<size_t> dist(0, m_MailWorkers.size() - 1);
			auto index = dist(gen);
			m_MailWorkers[index]->enqueue(toMailbox);
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

void MailHub::balance(MSDeque<MSRef<IMailBox>>& result) const
{
	thread_local std::mt19937 gen(std::random_device{}());
	std::uniform_int_distribution<size_t> dist(0, m_MailWorkers.size() - 1);
	auto index = dist(gen);
	if (m_MailWorkers[index]) m_MailWorkers[index]->balance(result);
}
