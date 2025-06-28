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

void RPCServer::startup()
{
	config_t config;
	configureEndpoint(config);

	if (config.Callback.OnOpen == nullptr)
	{
		config.Callback.OnOpen = [=](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addLast("default", MSNew<RPCServerInboundHandler>(this));
		};
	}
	m_Buffers = config.Buffers;
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
	MSMutexLock lock(m_Lock);
	return m_Methods.emplace(name, method).second;
}

bool RPCServer::unbind(MSStringView name)
{
	MSMutexLock lock(m_Lock);
	return m_Methods.erase(MSString(name));
}

bool RPCServer::invoke(MSStringView name, MSString const& input, MSString& output)
{
	decltype(m_Methods)::value_type::second_type method;
	{
		MSMutexLock lock(m_Lock);
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

			// Handle remote request message

			if (message.size() < m_Server->m_Buffers)
			{
				MSString output;
				RPCRequest request;
				if (TTypeC(message, request))
				{
					if (m_Server->invoke(request.Name, request.Args, output))
					{
						// Send the response message

						RPCResponse response;
						response.ID = request.ID;
						response.Args = output;
						auto _event = MSNew<IChannelEvent>();
						TTypeC(response, _event->Message);
						// Use '\0' to split the message
						_event->Message += char();
						context->writeAndFlush(_event);
					}
					else
					{
						context->close();
					}
				}
			}

			i = start;
		}

		m_Buffer = event->Message.substr(index + 1);
	}

	return false;
}
