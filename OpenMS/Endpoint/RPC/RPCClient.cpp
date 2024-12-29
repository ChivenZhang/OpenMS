/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "RPCClient.h"

void RPCClient::startup()
{
	config_t config;
	configureEndpoint(config);
	config.Callback.OnOpen = [=](MSRef<IChannel> channel)
	{
		channel->getPipeline()->addFirst("rpc", MSNew<RPCClientInboundHandler>(this));
	};
	m_Buffers = config.Buffers;
	m_Reactor = MSNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void RPCClient::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;
	m_Sessions.clear();
}

bool RPCClient::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool RPCClient::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> RPCClient::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}

RPCClientInboundHandler::RPCClientInboundHandler(MSRaw<RPCClient> client)
	:
	m_Client(client)
{
}

bool RPCClientInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	// Use '\0' to split the message
	auto index = event->Message.find(char());
	if (index == MSString::npos) m_Buffer += event->Message;
	else m_Buffer += event->Message.substr(0, index);
	if (m_Client->m_Buffers <= m_Buffer.size()) context->close();

	if (index != MSString::npos)
	{
		// Handle the response message

		RPCResponse response;
		if (TTypeC(m_Buffer, response))
		{
			MSMutexLock lock(m_Client->m_Lock);
			auto result = m_Client->m_Sessions.find(response.indx);
			if (result != m_Client->m_Sessions.end())
			{
				result->second.OnResult(std::move(response.args));
				m_Client->m_Sessions.erase(result);
			}
		}

		m_Buffer = event->Message.substr(index + 1);
	}
	return false;
}