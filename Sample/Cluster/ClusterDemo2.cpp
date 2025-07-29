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
	explicit AuthorMailbox(IMailContextRaw context) : MailBox(context)
	{
	}

	IMailTask read(IMail&& mail) override
	{
		static int count = 0;
		count++;
		static int t0 = ::clock();
		int t1 = ::clock();
		if (t0 + CLOCKS_PER_SEC <= t1)
		{
			MS_INFO("read %.0f mails per second", count * 1.0f * CLOCKS_PER_SEC / (t1 - t0));
			count = 0;
			t0 = t1;
		}

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
	ClusterService::onInit();

	auto mails = AUTOWIRE(IMailContext)::bean();
	mails->createMailbox<AuthorMailbox>("author");
}

void ClusterDemo2::onExit()
{
	ClusterService::onExit();
}