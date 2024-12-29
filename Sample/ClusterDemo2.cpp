/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 21:47:04.
*
* =================================================*/
#include "ClusterDemo2.h"

class AuthorMailbox : public MailBox
{
public:
	explicit AuthorMailbox(IMailContextRaw context) : MailBox(context)
	{
	}

	~AuthorMailbox() override
	{
	}

	IMailTask<void> read(IMail&& mail) override
	{
		MS_INFO("read mail: %s -> %s \"%s\"", mail.From.c_str(), mail.To.c_str(), mail.Data.c_str());
		co_return;
	}
};

// ========================================================================================

MSString ClusterDemo2::identity() const
{
	return "cluster2";
}

void ClusterDemo2::onInit()
{
	ClusterService::onInit();

	auto mails = AUTOWIRE(IMailContext)::bean();
	mails->createMailbox<AuthorMailbox>("author");
}

void ClusterDemo2::onExit()
{
	ClusterService::onExit();
}

int main(int argc, char* argv[])
{
	return IApplication::Run<ClusterDemo2>(argc, argv);
}