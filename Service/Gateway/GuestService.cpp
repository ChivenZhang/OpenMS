/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "GuestService.h"
#include "ProxyService.h"
#include "Reactor/IChannel.h"
#include "Mailbox/Private/Mail.h"

GuestService::GuestService(MSHnd<IChannel> client, uint32_t guestID)
	:
	m_GuestID(guestID),
	m_ClientChannel(client)
{
}

IMailTask GuestService::read(IMail mail)
{
	if (mail.Type & OPENMS_MAIL_TYPE_FORWARD)
	{
		mail.Type &= ~OPENMS_MAIL_TYPE_FORWARD;
		send(mail);
	}
	else if (mail.Type & OPENMS_MAIL_TYPE_REQUEST)
	{
		MS_INFO("%s\t%u => %u #%u %s", name().c_str(), mail.From, mail.To, mail.Date, mail.Body.substr(sizeof(uint32_t)).data());
		if (sizeof(request_t) <= mail.Body.size())
		{
			auto& request = *(request_t*)mail.Body.data();
			method_t method;
			{
				MSMutexLock lock(m_LockMethod);
				auto result = m_MethodMap.find(request.Method);
				if (result != m_MethodMap.end()) method = result->second;
			}
			MSString response;
			if (method)
			{
				auto input = MSStringView(request.Buffer, mail.Body.size() - sizeof(request_t));
				response = co_await method(input);

				std::swap(mail.From, mail.To);
				mail.Type &= ~OPENMS_MAIL_TYPE_REQUEST;
				mail.Type |= OPENMS_MAIL_TYPE_RESPONSE;
				mail.Body = response;
				if (mail.Copy != MSHash(nullptr)) mail.Type |= OPENMS_MAIL_TYPE_FORWARD;

				if (mail.Type & OPENMS_MAIL_TYPE_CLIENT)
				{
					if (auto client = m_ClientChannel.lock())
					{
						MSString buffer(sizeof(MailView) + mail.Body.size(), 0);
						auto& mailView = *(MailView*)buffer.data();
						mailView.From = mail.From;
						mailView.To = mail.To;
						mailView.Copy = mail.Copy;
						mailView.Date = mail.Date;
						mailView.Type = mail.Type;
						if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
						client->writeChannel(IChannelEvent::New(buffer));
					}
				}
				else
				{
					send(mail);
				}
			}
		}
	}
	else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
	{
		MS_INFO("%s\t%u <= %u 会话 %u 长度 %u 内容 %s", name().c_str(), mail.To, mail.From, mail.Date, (uint32_t)mail.Body.size(), mail.Body.data());
		session_t response;
		{
			MSMutexLock lock(m_LockSession);
			auto result = m_SessionMap.find(mail.Date);
			if (result != m_SessionMap.end())
			{
				response = result->second;
				m_SessionMap.erase(result);
			}
		}
		if (response) response(mail.Body);
	}
	co_return;
}