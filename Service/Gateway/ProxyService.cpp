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
#include "ProxyService.h"
#include "Reactor/IChannel.h"
#include "Mailbox/Private/Mail.h"

ProxyService::ProxyService(MSHnd<IChannel> client)
	:
	m_ClientChannel(client)
{
}

IMailTask ProxyService::read(IMail mail)
{
	// Handle client message
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
	// Handle server message
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
	co_return;
}