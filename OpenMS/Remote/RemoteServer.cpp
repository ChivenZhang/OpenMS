#include "RemoteServer.h"
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
#include "RemoteServer.h"

void RemoteServer::startup()
{
	config_t config;
	config.Callback =
	{
		[=](TRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("rpc", TNew<RemoteServerInboundHandler>(this));
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

void RemoteServer::shutdown()
{
	m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool RemoteServer::bind_internal(TStringView name, TLambda<bool(TString const&, TString&)> method)
{
	TMutexLock lock(m_Lock);
	return m_Methods.emplace(name, method).second;
}

bool RemoteServer::unbind(TStringView name)
{
	TMutexLock lock(m_Lock);
	return m_Methods.erase(TString(name));
}

bool RemoteServer::invoke(TStringView name, TString const& input, TString& output)
{
	TLambda<bool(TString const&, TString&)> method;
	{
		TMutexLock lock(m_Lock);
		auto result = m_Methods.find(TString(name));
		if (result == m_Methods.end()) return false;
		method = result->second;
	}
	if (method && method(input, output)) return true;
	return false;
}

struct RemoteServerRequest
{
	uint32_t indx;
	TString name;
	TString args;
	OPENMS_TYPE(RemoteServerRequest, indx, name, args)
};

struct RemoteServerResponse
{
	uint32_t indx;
	TString args;
	OPENMS_TYPE(RemoteServerResponse, indx, args)
};

RemoteServerInboundHandler::RemoteServerInboundHandler(TRaw<RemoteServer> server)
	:
	m_Server(server)
{
}

bool RemoteServerInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	// Use '\0' to split the message

	auto index = event->Message.find('\0');
	if (index == TString::npos) m_Buffer += event->Message;
	else m_Buffer += event->Message.substr(0, index);
	if (m_Server->m_Buffers <= m_Buffer.size()) context->close();

	if (index != TString::npos)
	{
		// Handle the request message

		TString output;
		RemoteServerRequest package;
		if (TTypeC(m_Buffer, package))
		{
			if (m_Server->invoke(package.name, package.args, output) == false)
			{
				context->close();
			}
		}

		// Send the response message

		RemoteServerResponse response;
		response.indx = package.indx;
		response.args = output;
		auto _event = TNew<IChannelEvent>();
		TTypeC(response, _event->Message);
		_event->Message += '\0';
		context->writeAndFlush(_event);

		m_Buffer = event->Message.substr(index + 1);
	}
	return false;
}