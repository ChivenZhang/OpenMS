/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
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
		static int count = 0;
		count++;

		static int t0 = ::clock();
		int t1 = ::clock();
		if (t0 + CLOCKS_PER_SEC <= t1)
		{
			MS_INFO("send %.0f mails per second", count * 1.0f * CLOCKS_PER_SEC / (t1 - t0));
			count = 0;
			t0 = t1;
		}

		send({.To = "author", .Data = mail.Data});
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

	m_Running = true;
	m_SendThread = MSThread([=]()
	{
		while (m_Running)
		{
			mails->sendToMailbox({.To = "login", .Data = R"({"user":"admin", "pass":"******"})"});
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});
}

void ClusterDemo1::onExit()
{
	m_Running = false;
	if (m_SendThread.joinable()) m_SendThread.join();

	ClusterService::onExit();
}

OPENMS_RUN(ClusterDemo1)
