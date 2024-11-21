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
#include "RPCClient.h"

void RPCClient::startup()
{
	config_t config;
	config.Callback =
	{
		[=](TRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("rpc", TNew<RPCClientInboundHandler>(this));
		},
	};
	configureEndpoint(config);
	m_Buffers = config.Buffers;
	m_Reactor = TNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("failed to start reactor");
}

void RPCClient::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
	m_Packages.clear();
}

RPCClientInboundHandler::RPCClientInboundHandler(TRaw<RPCClient> client)
	:
	m_Client(client)
{
}

bool RPCClientInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	// Use '\0' to split the message

	auto index = event->Message.find('\0');
	if (index == TString::npos) m_Buffer += event->Message;
	else m_Buffer += event->Message.substr(0, index);
	if (m_Client->m_Buffers <= m_Buffer.size()) context->close();

	if (index != TString::npos)
	{
		// Handle the response message

		RPCClientResponse response;
		if (TTypeC(m_Buffer, response))
		{
			TMutexLock lock(m_Client->m_Lock);
			auto result = m_Client->m_Packages.find(response.indx);
			if (result != m_Client->m_Packages.end())
			{
				result->second.OnResult(std::move(response.args));
				m_Client->m_Packages.erase(result);
			}
		}

		m_Buffer = event->Message.substr(index + 1);
	}
	return false;
}