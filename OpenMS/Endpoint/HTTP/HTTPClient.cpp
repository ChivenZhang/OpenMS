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
#include "HTTPClient.h"

HTTPClient::HTTPClient(config_t const& config)
	:
	m_Config(config)
{
}

void HTTPClient::startup()
{
	auto config = m_Config;
	if (config.Callback.OnOpen == nullptr)
	{
		config.Callback.OnOpen = [=](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addLast("default", MSNew<HTTPClientInboundHandler>(this));
		};
	}
	m_Reactor = MSNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		1,
		config.Callback
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void HTTPClient::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;
	while (m_Sessions.empty() == false) m_Sessions.pop();
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

bool HTTPClient::call_internal(request_t const& request, uint8_t type, uint32_t timeout, response_t& response)
{
	// Resolve request method type

	MSString method;
	switch (type)
	{
	case HTTP_DELETE: method = "DELETE";
		break;
	case HTTP_GET: method = "GET";
		break;
	case HTTP_POST: method = "POST";
		break;
	case HTTP_PUT: method = "PUT";
		break;
	default: return false;
	}

	// Generate request headers

	MSString headers;
	if (request.Header.find("Content-Type") == request.Header.end())
	{
		headers += "Content-Type" ":" "text/plain; charset=UTF-8" "\r\n";
	}
	if (request.Header.find("Content-Length") == request.Header.end())
	{
		headers += "Content-Length" ":" + std::to_string(request.Body.size()) + "\r\n";
	}
	if (request.Header.find("Connection") == request.Header.end())
	{
		headers += "Connection" ":" "keep-alive" "\r\n";
	}
	for (auto& header : request.Header) headers += header.first + ":" + header.second + "\r\n";

	// Generate request url params

	MSString params;
	for (auto& param : request.Params)
	{
		if (param.first.empty() == false)
		{
			if (params.empty() == false) params += "&";
			params += param.first;
			if (param.second.empty() == false) params += "=" + param.second;
		}
	}
	if (params.empty()) params = request.Url;
	else params = request.Url + "?" + params;

	// Send request to remote server

	if (timeout)
	{
		auto promise = MSNew<MSPromise<void>>();
		auto future = promise->get_future();
		{
			MSMutexLock lock(m_Lock);
			auto& session = m_Sessions.emplace();
			session.OnResult = [&, promise](response_t&& _response)
			{
				response = _response;
				promise->set_value();
			};
		}

		auto _request = method + " " + params + " " "HTTP/1.1" "\r\n" + headers + "\r\n" + request.Body;
		m_Reactor->write(IChannelEvent::New(_request));

		auto status = future.wait_for(std::chrono::milliseconds(timeout));
		if (status == std::future_status::ready) return true;
	}
	else
	{
		auto _request = method + " " + params + " " "HTTP/1.1" "\r\n" + headers + "\r\n" + request.Body;
		m_Reactor->write(IChannelEvent::New(_request));
		return m_Reactor->running();
	}

	return false;
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
	// Parse response from server

	m_Settings.on_message_begin = [](http_parser* parser)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_LastField = MSString();
		handler->m_Response.Code = 0;
		handler->m_Response.Header.clear();
		handler->m_Response.Body = MSString();
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
		auto result = handler->m_Response.Header.emplace(handler->m_LastField, MSString(at, length));
		if (result.second == false) result.first->second += "," + MSString(at, length);
		return 0;
	};
	m_Settings.on_body = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_Response.Body += MSString(at, length);
		return 0;
	};
	m_Settings.on_message_complete = [](http_parser* parser)
	{
		auto handler = (HTTPClientInboundHandler*)parser->data;
		handler->m_Response.Code = parser->status_code;
		return 0;
	};

	auto parsedNum = http_parser_execute(&m_Parser, &m_Settings, event->Message.data(), event->Message.size());
	if (parsedNum == event->Message.size())
	{
		if (m_Response.Code)
		{
			HTTPClient::invoke_t session;
			{
				MSMutexLock lock(m_Client->m_Lock);
				if (m_Client->m_Sessions.empty() == false)
				{
					session = std::move(m_Client->m_Sessions.front());
					m_Client->m_Sessions.pop();
				}
			}
			session.OnResult(std::move(m_Response));
			return true;
		}
	}
	else
	{
		context->close();
	}
	return false;
}
