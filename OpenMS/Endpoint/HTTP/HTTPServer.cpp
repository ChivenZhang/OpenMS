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
#include "HTTPServer.h"

void HTTPServer::startup()
{
	config_t config;
	configureEndpoint(config);
	if (config.Callback.OnOpen == nullptr)
	{
		config.Callback.OnOpen = [=](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addLast("default", MSNew<HTTPServerInboundHandler>(this));
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
	if (m_Reactor->running() == false)
		MS_FATAL("failed to start reactor");
}

void HTTPServer::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;
}

bool HTTPServer::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool HTTPServer::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> HTTPServer::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}

bool HTTPServer::bind(uint8_t method, MSString path, callback_t callback)
{
	switch (method)
	{
	case HTTP_DELETE:
		{
			MSMutexLock lock(m_LockDelete);
			m_DeleteMethods[path] = {HTTP_DELETE, callback};
		}
		break;
	case HTTP_GET:
		{
			MSMutexLock lock(m_LockGet);
			m_GetMethods[path] = {HTTP_GET, callback};
		}
		break;
	case HTTP_POST:
		{
			MSMutexLock lock(m_LockPost);
			m_PostMethods[path] = {HTTP_POST, callback};
		}
		break;
	case HTTP_PUT:
		{
			MSMutexLock lock(m_LockPut);
			m_PutMethods[path] = {HTTP_PUT, callback};
		}
		break;
	default:
		MS_WARN("unknown method %d", method);
		return false;
	}
	return true;
}

int on_message_begin(http_parser* parser)
{
	printf("Message Begin\n");
	return 0;
}

int on_headers_complete(http_parser* parser)
{
	printf("Headers Complete\n");
	return 0;
}

int on_message_complete(http_parser* parser)
{
	printf("Message Complete\n");
	return 0;
}

HTTPServerInboundHandler::HTTPServerInboundHandler(MSRaw<HTTPServer> server)
	:
	m_Server(server), m_Parser(), m_Settings()
{
	http_parser_init(&m_Parser, HTTP_REQUEST);
	http_parser_settings_init(&m_Settings);
	m_Parser.data = this;
}

bool HTTPServerInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	m_Settings.on_message_begin = [](http_parser* parser)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		handler->m_LastField = MSString();
		handler->m_Request.Url = MSString();
		handler->m_Request.Header.clear();
		handler->m_Request.Body = MSString();

		handler->m_Response.Code = 0;
		handler->m_Response.Header.clear();
		handler->m_Response.Body = MSString();
		return 0;
	};
	m_Settings.on_url = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		handler->m_Request.Url = MSString(at, length);
		return 0;
	};
	m_Settings.on_header_field = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		handler->m_LastField = MSString(at, length);
		return 0;
	};
	m_Settings.on_header_value = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		handler->m_Request.Header[handler->m_LastField] = MSString(at, length);
		return 0;
	};
	m_Settings.on_headers_complete = [](http_parser* parser)
	{
		return 0;
	};
	m_Settings.on_body = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		handler->m_Request.Body = MSString(at, length);
		return 0;
	};
	m_Settings.on_message_complete = [](http_parser* parser)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;

		HTTPServer::callback_t method;

		switch (parser->method)
		{
		case HTTP_DELETE:
			{
				MSMutexLock lock(handler->m_Server->m_LockDelete);
				auto result = handler->m_Server->m_DeleteMethods.find(handler->m_Request.Url);
				if (result != handler->m_Server->m_DeleteMethods.end() && result->second.Method == HTTP_DELETE) method = result->second.Callback;
			}
			break;
		case HTTP_GET:
			{
				MSMutexLock lock(handler->m_Server->m_LockGet);
				auto result = handler->m_Server->m_GetMethods.find(handler->m_Request.Url);
				if (result != handler->m_Server->m_GetMethods.end() && result->second.Method == HTTP_GET) method = result->second.Callback;
			}
			break;
		case HTTP_POST:
			{
				MSMutexLock lock(handler->m_Server->m_LockPost);
				auto result = handler->m_Server->m_PostMethods.find(handler->m_Request.Url);
				if (result != handler->m_Server->m_PostMethods.end() && result->second.Method == HTTP_POST) method = result->second.Callback;
			}
			break;
		case HTTP_PUT:
			{
				MSMutexLock lock(handler->m_Server->m_LockPut);
				auto result = handler->m_Server->m_PutMethods.find(handler->m_Request.Url);
				if (result != handler->m_Server->m_PutMethods.end() && result->second.Method == HTTP_PUT) method = result->second.Callback;
			}
			break;
		default:
			break;
		}

		if (method)
		{
			method(handler->m_Request, handler->m_Response);
			if (handler->m_Response.Code == 0)
			{
				handler->m_Response.Code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
				handler->m_Response.Body = "Internal Server Error";
			}
			return 0;
		}

		if (handler->m_Response.Code == 0)
		{
			handler->m_Response.Code = HTTP_STATUS_NOT_FOUND;
			handler->m_Response.Body = "Not Found";
		}
		return 0;
	};

	auto parsedNum = http_parser_execute(&m_Parser, &m_Settings, event->Message.data(), event->Message.size());
	if (parsedNum == event->Message.size())
	{
		if (m_Response.Code)
		{
			MSString contentType = m_Response.ContentType;
			MSString contentLength = std::to_string(m_Response.Body.size());
			MSString status_line = "HTTP/1.0 " + std::to_string(m_Response.Code);
			MSString headers = "Content-Type: " + contentType + "\r\nContent-Length: " + contentLength + "\r\n";
			MSString& body = m_Response.Body;
			auto response = status_line + "\r\n" + headers + "\r\n" + body;
			context->writeAndFlush(IChannelEvent::New(response));
		}
	}
	else
	{
		context->close();
	}
	return false;
}
