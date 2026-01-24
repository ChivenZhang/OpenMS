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
	if (mail.Type & OPENMS_MAIL_TYPE_DOMAIN)
	{
		mail.Type &= ~OPENMS_MAIL_TYPE_DOMAIN;

		if (mail.Type & OPENMS_MAIL_TYPE_CLIENT)
		{
			if (mail.From != MSHash("client:" + std::to_string(m_UserID)))
			{
				MS_WARN("p2p message from user %u", m_UserID);
				co_return;
			}
			if (mail.Type & OPENMS_MAIL_TYPE_REQUEST)
			{
				send(mail);
			}
			else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
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
		}
		else
		{
			if (mail.Type & OPENMS_MAIL_TYPE_REQUEST)
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
			else if (mail.Type & OPENMS_MAIL_TYPE_RESPONSE)
			{
				send(mail);
			}
		}
	}
	co_return;
}