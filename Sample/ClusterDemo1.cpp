/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 19:54:44.
*
* =================================================*/
#include "ClusterDemo1.h"

class LoginMailbox : public MailBox
{
public:
	explicit LoginMailbox(IMailContextRaw context) : MailBox(context)
	{
	}

	IMailTask<void> read(IMail&& mail) override
	{
		MS_INFO("send mail to author...");
		send({ .To = "author", .Data = mail.Data });
		co_return;
	}
};

// ========================================================================================

MSString ClusterDemo1::identity() const
{
	return "cluster1";
}

void ClusterDemo1::onInit()
{
	ClusterService::onInit();

	auto mails = AUTOWIRE(IMailContext)::bean();
	mails->createMailbox<LoginMailbox>("login");

	startTimer(0, 2000, [=](uint32_t handle)
	{
		mails->sendToMailbox({.To = "login", .Data = R"({"user":"admin", "pass":"******"})" });
	});
}

void ClusterDemo1::onExit()
{
	ClusterService::onExit();
}

int main(int argc, char* argv[])
{
	return IApplication::Run<ClusterDemo1>(argc, argv);
}
