#pragma once
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
#include <OpenMS/Server/Private/Server.h>
#include <OpenMS/Endpoint/TCP/TCPClient.h>
#include <OpenMS/Mailbox/Private/MailHub.h>

class FrontendClient : public Server
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	void onUpdate(float time) override;

protected:
	MSRef<MailHub> m_MailHub;
	MSRef<TCPClient> m_TCPClient;
	MSRef<IChannel> m_TCPChannel;
};

OPENMS_RUN(FrontendClient);