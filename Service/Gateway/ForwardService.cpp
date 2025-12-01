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
#include "ForwardService.h"
#include "Reactor/IChannel.h"
#include "Mailbox/Private/Mail.h"

ForwardService::ForwardService(MSHnd<IChannel> client)
	:
	m_ClientChannel(client)
{
}

IMailTask ForwardService::read(IMail mail)
{
	if (mail.Type == 0)
	{
		if (sizeof(request_t) <= mail.Body.size())
		{
			auto& request = *(request_t*)mail.Body.data();
			method_t method;
			{
				MSMutexLock lock(m_MutexMethod);
				auto result = m_MethodMap.find(request.Method);
				if (result != m_MethodMap.end()) method = result->second;
			}
			if (method)
			{
				auto input = MSStringView(request.Buffer, mail.Body.size() - sizeof(request_t));
				co_await method(input);
			}
		}
	}
	else
	{
		if (mail.Type & OPENMS_MAIL_TYPE_CLIENT)
		{
			if (mail.Type & OPENMS_MAIL_TYPE_REQUEST)
			{
				send(mail);
			}
			else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
			{
				if (auto client = m_ClientChannel.lock())
				{
					auto userID = (uint32_t)client->getContext()->userdata();
					MSString buffer(sizeof(MailView) + mail.Body.size(), 0);
					auto& mailView = *(MailView*)buffer.data();
					mailView.From = name();
					mailView.To = MSHash("client:" + std::to_string(userID));
					mailView.Date = mail.Date;
					mailView.Type = mail.Type;
					if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
					client->writeChannel(IChannelEvent::New(buffer));
				}
			}
		}
		else
		{
			if (mail.Type & OPENMS_MAIL_TYPE_REQUEST)
			{
				if (auto client = m_ClientChannel.lock())
				{
					auto userID = (uint32_t)client->getContext()->userdata();
					MSString buffer(sizeof(MailView) + mail.Body.size(), 0);
					auto& mailView = *(MailView*)buffer.data();
					mailView.From = name();
					mailView.To = MSHash("client:" + std::to_string(userID));
					mailView.Date = mail.Date;
					mailView.Type = mail.Type;
					if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
					client->writeChannel(IChannelEvent::New(buffer));
				}
			}
			else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
			{
				send(mail);
			}
		}
	}
	co_return;
}