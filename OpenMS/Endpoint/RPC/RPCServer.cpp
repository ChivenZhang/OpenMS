#include "RPCServer.h"
#include "RPCServer.h"
#include "RPCServer.h"
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
#include "RPCServer.h"

void RPCServer::startup()
{
	config_t config;
	config.Callback =
	{
		[=](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("rpc", MSNew<RPCServerInboundHandler>(this));
		},
	};
	configureEndpoint(config);
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
	m_Reactor->shutdown();
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
	// Use '\0' to split the message
	auto index = event->Message.find(char());
	if (index == MSString::npos) m_Buffer += event->Message;
	else m_Buffer += event->Message.substr(0, index);
	if (m_Server->m_Buffers <= m_Buffer.size()) context->close();

	if (index != MSString::npos)
	{
		// Handle the request message

		MSString output;
		RPCServerRequest request;
		if (TTypeC(m_Buffer, request))
		{
			if (m_Server->invoke(request.name, request.args, output) == false)
			{
				context->close();
			}
		}

		// Send the response message

		RPCServerResponse response;
		response.indx = request.indx;
		response.args = output;
		auto _event = MSNew<IChannelEvent>();
		TTypeC(response, _event->Message);
		// Use '\0' to split the message
		_event->Message += char();
		context->writeAndFlush(_event);

		m_Buffer = event->Message.substr(index + 1);
	}
	return false;
}