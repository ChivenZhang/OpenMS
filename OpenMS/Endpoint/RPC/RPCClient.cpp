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
#include "Reactor/Private/ChannelHandler.h"

class RPCClientInboundHandler : public ChannelInboundHandler
{
public:
	explicit RPCClientInboundHandler(MSRaw<RPCClient> client);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<RPCClient> m_Client;
};

RPCClient::RPCClient(config_t const& config)
	:
	m_Config(config)
{
}

void RPCClient::startup()
{
	auto config = m_Config;
	m_Reactor = MSNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		TCPClientReactor::callback_tcp_t
		{
			.OnOpen = [this](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addLast("default", MSNew<RPCClientInboundHandler>(this));
			},
		}
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

	struct stream_t
	{
		uint32_t Length;
		char Buffer[0];
	};
	auto& stream = *(stream_t*)m_Buffer.data();

	if (sizeof(stream_t) <= m_Buffer.size() && sizeof(stream_t) + stream.Length <= m_Buffer.size())
	{
		auto message = MSStringView(stream.Buffer, stream.Length);
		if (message.size() <= m_Client->m_Config.Buffers)
		{
			auto& responseView = *(RPCResponseView*)message.data();
			if (sizeof(RPCResponseView) <= message.size() && sizeof(RPCResponseView) + responseView.Length == message.size())
			{
				decltype(m_Client->m_Sessions)::value_type::second_type callback;
				{
					MSMutexLock lock(m_Client->m_Locker);
					auto result = m_Client->m_Sessions.find(responseView.ID);
					if (result != m_Client->m_Sessions.end())
					{
						callback = result->second;
						m_Client->m_Sessions.erase(result);
					}
				}
				if (callback) callback(MSStringView(responseView.Buffer, responseView.Length));
			}
		}

		m_Buffer = m_Buffer.substr(sizeof(uint32_t) + stream.Length);
	}
	return false;
}
