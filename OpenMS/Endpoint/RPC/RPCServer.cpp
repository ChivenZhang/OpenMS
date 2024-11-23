#include "RPCServer.h"
#include "RPCServer.h"
#include "RPCServer.h"
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
#include "RPCServer.h"

void RPCServer::startup()
{
	config_t config;
	config.Callback =
	{
		[=](TRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("rpc", TNew<RPCServerInboundHandler>(this));
		},
	};
	configureEndpoint(config);
	m_Buffers = config.Buffers;
	m_Reactor = TNew<TCPServerReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Backlog,
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("failed to start reactor");
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

THnd<IChannelAddress> RPCServer::address() const
{
	return m_Reactor ? m_Reactor->address() : THnd<IChannelAddress>();
}

bool RPCServer::bind_internal(TStringView name, TLambda<bool(TString const&, TString&)> method)
{
	TMutexLock lock(m_Lock);
	return m_Methods.emplace(name, method).second;
}

bool RPCServer::unbind(TStringView name)
{
	TMutexLock lock(m_Lock);
	return m_Methods.erase(TString(name));
}

bool RPCServer::invoke(TStringView name, TString const& input, TString& output)
{
	decltype(m_Methods)::value_type::second_type method;
	{
		TMutexLock lock(m_Lock);
		auto result = m_Methods.find(TString(name));
		if (result == m_Methods.end()) return false;
		method = result->second;
	}
	if (method && method(input, output)) return true;
	return false;
}

RPCServerInboundHandler::RPCServerInboundHandler(TRaw<RPCServer> server)
	:
	m_Server(server)
{
}

bool RPCServerInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	// Use '\0' to split the message
	auto index = event->Message.find(char());
	if (index == TString::npos) m_Buffer += event->Message;
	else m_Buffer += event->Message.substr(0, index);
	if (m_Server->m_Buffers <= m_Buffer.size()) context->close();

	if (index != TString::npos)
	{
		// Handle the request message

		TString output;
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
		auto _event = TNew<IChannelEvent>();
		TTypeC(response, _event->Message);
		// Use '\0' to split the message
		_event->Message += char();
		context->writeAndFlush(_event);

		m_Buffer = event->Message.substr(index + 1);
	}
	return false;
}