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
#include "HTTPClient.h"

void HTTPClient::startup()
{
	config_t config;
	configureEndpoint(config);
	if (config.Callback.OnOpen == nullptr)
	{
		config.Callback.OnOpen = [=](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addLast("default", MSNew<HTTPClientInboundHandler>(this));
		};
	}
	m_Buffers = config.Buffers;
	m_Reactor = MSNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false)
		MS_FATAL("failed to start reactor");
}

void HTTPClient::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool HTTPClient::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool HTTPClient::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> HTTPClient::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}

HTTPClientInboundHandler::HTTPClientInboundHandler(MSRaw<HTTPClient> client)
	:
	m_Client(client), m_Parser(), m_Settings()
{
	http_parser_init(&m_Parser, HTTP_RESPONSE);
	http_parser_settings_init(&m_Settings);
	m_Parser.data = this;
}

bool HTTPClientInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	m_Settings.on_message_begin = [](http_parser* parser)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_LastField = MSString();
		handler->m_Request.Url = MSString();
		handler->m_Request.Header.clear();
		handler->m_Request.Body = MSString();

		handler->m_Response.Code = 404;
		handler->m_Response.Header.clear();
		handler->m_Response.Body = MSString();
		return 0;
	};
	m_Settings.on_url = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_Request.Url = MSString(at, length);
		return 0;
	};
	m_Settings.on_header_field = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_LastField = MSString(at, length);
		return 0;
	};
	m_Settings.on_header_value = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_Request.Header[handler->m_LastField] = MSString(at, length);
		return 0;
	};
	m_Settings.on_headers_complete = [](http_parser* parser)
	{
		return 0;
	};
	m_Settings.on_body = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_Request.Body = MSString(at, length);
		return 0;
	};
	m_Settings.on_message_complete = [](http_parser* parser)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;

		HTTPClient::callback_t method;

		if (method)
		{
			method(handler->m_Request, handler->m_Response);
			return 0;
		}

		MS_INFO("error %s", handler->m_Request.Url.c_str());
		return 0;
	};

	auto parsedNum = http_parser_execute(&m_Parser, &m_Settings, event->Message.data(), event->Message.size());
	if (parsedNum != event->Message.size())
	{
		context->close();
	}
	return false;
}
