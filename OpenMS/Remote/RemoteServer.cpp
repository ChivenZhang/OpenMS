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
	configureEndpoint(config);
	config.Workers = 0;
	config.Callback =
	{
		[=](TRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("", TNew<RemoteServerInboundHandler>(this));
		},
	};
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
	return m_Methods.emplace(name, method).second;
}

bool RemoteServer::unbind(TStringView name)
{
	return m_Methods.erase(TString(name));
}

bool RemoteServer::invoke(TStringView name, TString const& input, TString& output)
{
	auto result = m_Methods.find(TString(name));
	if (result == m_Methods.end()) return false;

	auto method = result->second;
	if (method(input, output) == false) return false;

	return true;
}

struct RemoteServerRequest
{
	TString name;
	TString args;
	OPENMS_TYPE(RemoteServerRequest, name, args)
};

RemoteServerInboundHandler::RemoteServerInboundHandler(TRaw<RemoteServer> server)
	:
	m_Server(server)
{
}

bool RemoteServerInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	auto index = event->Message.find('\0');
	if (index == TString::npos) m_Buffer += event->Message;
	else m_Buffer += event->Message.substr(0, index);
	if (UINT_MAX <= m_Buffer.size()) context->close();

	if (index != TString::npos)
	{
		TString output;
		RemoteServerRequest package;
		if (TTypeC(m_Buffer, package))
		{
			m_Server->invoke(package.name, package.args, output);
		}
		auto _event = TNew<IChannelEvent>();
		_event->Message = output + '\0';
		context->writeAndFlush(_event);

		m_Buffer = m_Buffer.substr(index + 1);
	}
	return false;
}