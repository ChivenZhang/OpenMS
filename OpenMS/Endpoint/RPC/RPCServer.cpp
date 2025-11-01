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
#include "RPCServer.h"

class RPCServerInboundHandler : public ChannelInboundHandler
{
public:
	explicit RPCServerInboundHandler(MSRaw<RPCServer> server);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<RPCServer> m_Server;
};

RPCServer::RPCServer(config_t const& config)
	:
	m_Config(config)
{
}

void RPCServer::startup()
{
	auto config = m_Config;
	m_Reactor = MSNew<TCPServerReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Backlog,
		config.Workers,
		TCPServerReactor::callback_tcp_t
		{
			.OnOpen = [this](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addLast("default", MSNew<RPCServerInboundHandler>(this));
			},
		}
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void RPCServer::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool RPCServer::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool RPCServer::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> RPCServer::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}

bool RPCServer::bind_internal(MSStringView name, method_t&& method)
{
	MSMutexLock lock(m_Locker);
	return m_Methods.emplace(MSHash(name), method).second;
}

bool RPCServer::unbind(MSStringView name)
{
	MSMutexLock lock(m_Locker);
	return m_Methods.erase(MSHash(name));
}

bool RPCServer::invoke(uint32_t hash, MSStringView const& input, MSString& output)
{
	decltype(m_Methods)::value_type::second_type method;
	{
		MSMutexLock lock(m_Locker);
		auto result = m_Methods.find(hash);
		if (result == m_Methods.end()) return false;
		method = result->second;
	}
	if (method && method(input, output)) return true;
	return false;
}

RPCServerInboundHandler::RPCServerInboundHandler(MSRaw<RPCServer> server)
	:
	m_Server(server)
{
}

bool RPCServerInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
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
		if (message.size() <= m_Server->m_Config.Buffers)
		{
			auto& requestView = *(RPCRequestView*)message.data();
			if (sizeof(RPCRequestView) <= message.size() && sizeof(RPCRequestView) + requestView.Length == message.size())
			{
				MSString response;
				if (m_Server->invoke(requestView.Name, MSStringView(requestView.Buffer, requestView.Length), response))
				{
					MSString output(sizeof(RPCResponseView) + response.size(), 0);
					RPCResponseView& responseView = *(RPCResponseView*)output.data();
					responseView.ID = requestView.ID;
					responseView.Length = (uint32_t)response.size();
					if (responseView.Length) ::memcpy(responseView.Buffer, response.data(), response.size());

					auto length = (uint32_t)output.size();
					context->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output));
				}
				else
				{
					context->close();
				}
			}
		}

		m_Buffer = m_Buffer.substr(sizeof(uint32_t) + stream.Length);
	}

	return false;
}
