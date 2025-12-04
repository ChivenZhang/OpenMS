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

ProxyService::ProxyService(MSHnd<IChannel> client, uint32_t userID)
	:
	m_UserID(userID),
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
			mail.To = MSHash("player:" + std::to_string(m_UserID));
			send(mail);
		}
		else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
		{
			if (auto client = m_ClientChannel.lock())
			{
				MSString buffer(sizeof(MailView) + mail.Body.size(), 0);
				auto& mailView = *(MailView*)buffer.data();
				mailView.From = MSHash("proxy:" + std::to_string(m_UserID));
				mailView.To = MSHash("client:" + std::to_string(m_UserID));
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
				MSString buffer(sizeof(MailView) + mail.Body.size(), 0);
				auto& mailView = *(MailView*)buffer.data();
				mailView.From = mail.From;
				mailView.To = MSHash("client:" + std::to_string(m_UserID));
				mailView.Date = mail.Date;
				mailView.Type = mail.Type;
				if (mail.Body.empty() == false) ::memcpy(mailView.Body, mail.Body.data(), mail.Body.size());
				client->writeChannel(IChannelEvent::New(buffer));
			}
		}
		else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
		{
			mail.From = MSHash("proxy:" + std::to_string(m_UserID));
			send(mail);
		}
	}
	co_return;
}