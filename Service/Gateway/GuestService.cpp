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
	this->bind("login", [=, this](MSString username, MSString password)-> MSAsync<uint32_t>
	{
		MS_INFO("登录请求： %s", username.c_str());

		auto channel = m_ClientChannel.lock();
		if (channel == nullptr) co_return 0U;
		if (auto userID = channel->getContext()->userdata()) co_return userID;
		auto userID = co_await this->async<uint32_t>("logic", "login", "", 1000, MSTuple{username, password});
		if (userID)
		{
			auto proxyService = MSNew<ProxyService>(channel, userID);
			auto result = this->create("proxy:" + std::to_string(userID), proxyService);
			if (result == false) co_return 0U;

			channel->getContext()->userdata() = userID;
			MS_INFO("验证成功！ %s", username.c_str());
			co_await this->async<void>("client", "onLogin", proxyService->name(), 0, MSTuple{ userID });
		}
		else
		{
			MS_INFO("验证失败！ %s", username.c_str());
		}

		co_return userID;
	});
	this->bind("logout", [=, this]()-> MSAsync<bool>
	{
		MS_INFO("注销请求");

		auto channel = m_ClientChannel.lock();
		if (channel == nullptr) co_return false;
		auto userID = channel->getContext()->userdata();
		if (userID == 0) co_return false;
		auto result = co_await this->async<bool>("logic", "logout", "", 1000, MSTuple{userID});

		co_await this->async<void>("client", "onLogout", this->name(), 0, MSTuple{result});
		co_return result;
	});
	this->bind("signup", [=, this](MSString username, MSString password)-> MSAsync<bool>
	{
		MS_INFO("注册请求： %s", username.c_str());

		auto userID = co_await this->async<uint32_t>("logic", "signup", "", 1000, MSTuple{username, password});

		co_await this->async<void>("client", "onSignup", this->name(), 0, MSTuple{userID});
		co_return userID;
	});
}

IMailTask GuestService::read(IMail mail)
{
	if (mail.Type & OPENMS_MAIL_TYPE_FORWARD)
	{
		mail.Type &= ~OPENMS_MAIL_TYPE_FORWARD;

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
	else
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
				if (method)
				{
					auto input = MSStringView(request.Buffer, mail.Body.size() - sizeof(request_t));
					auto response = co_await method(input);

					std::swap(mail.From, mail.To);
					mail.Type &= ~OPENMS_MAIL_TYPE_REQUEST;
					mail.Type |= OPENMS_MAIL_TYPE_RESPONSE;
					mail.Body = response;
					if (mail.Copy != MSHash(nullptr)) mail.Type |= OPENMS_MAIL_TYPE_FORWARD;

					if (mail.Type & OPENMS_MAIL_TYPE_CLIENT)
					{
						if (auto client = m_ClientChannel.lock())
						{
							MSString buffer(sizeof(MailView) + mail.Body.size(), '?');
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
	}
	co_return;
}