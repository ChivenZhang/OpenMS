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
#include "RPCClient.h"

void RPCClient::startup()
{
	config_t config;
	configureEndpoint(config);

	if (config.Callback.OnOpen == nullptr)
	{
		config.Callback.OnOpen = [=](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addLast("default", MSNew<RPCClientInboundHandler>(this));
		};
	}
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
	m_Buffer += event->Message;

	// Use '\0' to split the message
	auto index = event->Message.find(char());
	if (index != MSString::npos)
	{
		for (size_t i = 0; i < m_Buffer.size(); ++i)
		{
			// Use '\0' to split the message
			auto start = m_Buffer.find(char(), i);
			if (start == MSString::npos) break;
			auto message = MSStringView(m_Buffer.data() + i, start - i);

			// Handle remote response message

			if (message.size() < m_Client->m_Buffers)
			{
				RPCResponse response;
				if (TTypeC(message, response))
				{
					MSMutexLock lock(m_Client->m_Lock);
					auto result = m_Client->m_Sessions.find(response.ID);
					if (result != m_Client->m_Sessions.end())
					{
						result->second.OnResult(std::move(response.Args));
						m_Client->m_Sessions.erase(result);
					}
				}
			}

			i = start;
		}

		m_Buffer = event->Message.substr(index + 1);
	}

	return false;
}
