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
#include "ClusterDemo.h"

class LoginMailbox : public MailBox
{
public:
	explicit LoginMailbox(IMailContextRaw context) : MailBox(context)
	{
	}

	~LoginMailbox() override
	{
	}

	IMailTask<void> read(IMail&& mail) override
	{
		MS_INFO("send mail to author...");
		send({ .To = "author", .Data = "login..." });
		co_return;
	}
};

// ========================================================================================

void ClusterDemo::onInit()
{
	ClusterService::onInit();

	auto mails = AUTOWIRE(IMailContext)::bean();
	mails->createMailbox<LoginMailbox>("login");
	mails->sendToMailbox({.To = "login", .Data = R"({"user":"admin", "pass":"******"})" });
}

void ClusterDemo::onExit()
{
	ClusterService::onExit();
}

int main(int argc, char* argv[])
{
	return IApplication::Run<ClusterDemo>(argc, argv);
}
