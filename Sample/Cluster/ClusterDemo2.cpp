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
#include "ClusterDemo2.h"

class AuthorMailbox : public MailBox
{
public:
	using MailBox::MailBox;

	IMailTask read(IMail mail) override
	{
		auto newMail = mail;
		newMail.Body = "success";
		newMail.To = mail.From;
		send(newMail);
		MS_INFO("success");
		co_return;
	}
};

// ========================================================================================

MSString ClusterDemo2::identity() const
{
	// Use config in Application.json
	return "cluster2";
}

void ClusterDemo2::onInit()
{
	ClusterServer::onInit();

	auto hub = AUTOWIRE(IMailHub)::bean();
	hub->create<AuthorMailbox>("author");
}

void ClusterDemo2::onExit()
{
	ClusterServer::onExit();
}