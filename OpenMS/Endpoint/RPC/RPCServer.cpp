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
	if (config.Callback.OnOpen == nullptr)
	{
		config.Callback.OnOpen = [this](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addLast("default", MSNew<RPCServerInboundHandler>(this));
		};
	}
	m_Reactor = MSNew<TCPServerReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Backlog,
		config.Workers,
		config.Callback
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

bool RPCServer::bind_internal(MSStringView name, MSLambda<bool(MSString const&, MSString&)> method)
{
	MSMutexLock lock(m_Locker);
	return m_Methods.emplace(name, method).second;
}

bool RPCServer::unbind(MSStringView name)
{
	MSMutexLock lock(m_Locker);
	return m_Methods.erase(MSString(name));
}

bool RPCServer::invoke(MSStringView name, MSString const& input, MSString& output)
{
	decltype(m_Methods)::value_type::second_type method;
	{
		MSMutexLock lock(m_Locker);
		auto result = m_Methods.find(MSString(name));
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

	if (sizeof(uint32_t) <= m_Buffer.size() && sizeof(uint32_t) + stream.Length <= m_Buffer.size())
	{
		auto message = MSStringView(stream.Buffer, stream.Length);

		if (message.size() <= m_Server->m_Config.Buffers)
		{
			RPCRequest request;
			if (TTypeC(message, request))
			{
				MSString output;
				if (m_Server->invoke(request.Name, request.Args, output))
				{
					RPCResponse response;
					response.ID = request.ID;
					response.Args = output;
					if (TTypeC(response, output))
					{
						auto length = (uint32_t)output.size();
						context->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output));
					}
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
