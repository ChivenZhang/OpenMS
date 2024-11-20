/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "RemoteClient.h"

void RemoteClient::startup()
{
	config_t config;
	configureEndpoint(config);
	config.Workers = 0;
	config.Callback =
	{
		[=](TRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("", TNew<RemoteClientInboundHandler>(this));
		},
	};
	m_Reactor = TNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("failed to start reactor");
}

void RemoteClient::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}

RemoteClientInboundHandler::RemoteClientInboundHandler(TRaw<RemoteClient> server)
	:
	m_Client(server)
{
}

bool RemoteClientInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	auto index = event->Message.find('\0');
	if (index == TString::npos) m_Buffer += event->Message;
	else m_Buffer += event->Message.substr(0, index);
	if (UINT_MAX <= m_Buffer.size()) context->close();

	if (index != TString::npos)
	{
		TString output = m_Buffer;

		// TODO: async message handling
		TPrint("recv: %s", output.c_str());

		m_Buffer = m_Buffer.substr(index + 1);
	}
	return false;
}