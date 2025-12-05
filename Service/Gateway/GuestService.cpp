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
	if (mail.Type & OPENMS_MAIL_TYPE_REQUEST)
	{
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
			}
			mail.Type &= ~OPENMS_MAIL_TYPE_REQUEST;
			mail.Type |= OPENMS_MAIL_TYPE_RESPONSE;
			mail.Body = response;
			std::swap(mail.From, mail.To);

			if (mail.Type & OPENMS_MAIL_TYPE_CLIENT)
			{
				if (auto client = m_ClientChannel.lock())
				{
					auto userID = (uint32_t)client->getContext()->userdata();
					MSString buffer(sizeof(MailView) + mail.Body.size(), 0);
					auto& mailView = *(MailView*)buffer.data();
					mailView.From = mail.From;
					mailView.To = mail.To;
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
	else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
	{
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