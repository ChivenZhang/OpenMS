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
#include "ClusterDemo1.h"

class LoginMailbox : public MailBox
{
public:
	using MailBox::MailBox;

	IMailTask read(IMail mail) override
	{
		if (mail.From == MSHash("author"))
		{
			MS_INFO("get %s", MSString(mail.Body).c_str());
		}
		else
		{
			send({.To = MSHash("author"), .Body = mail.Body });
		}
		co_return;
	}

	MSSet<uint32_t> m_Session;
};

// ========================================================================================

MSString ClusterDemo1::identity() const
{
	// Use config in Application.json
	return "cluster1";
}

void ClusterDemo1::onInit()
{
	ClusterServer::onInit();

	auto hub = AUTOWIRE(IMailHub)::bean();
	hub->create<LoginMailbox>("login");

	m_Running = true;
	m_Thread = MSThread([=, this]()
	{
		while (m_Running)
		{
			hub->send({.To = MSHash("login"), .Body = R"({"user":"admin", "pass":"******"})"});
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	});
}

void ClusterDemo1::onExit()
{
	m_Running = false;
	if (m_Thread.joinable()) m_Thread.join();

	ClusterServer::onExit();
}
