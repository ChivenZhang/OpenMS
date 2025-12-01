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
#include "HTTPServer.h"
#include <regex>
#include <cpptrace.hpp>

HTTPServer::HTTPServer(config_t const& config)
	:
	m_Config(config)
{
}

void HTTPServer::startup()
{
	auto config = m_Config;
	if (config.Callback.OnOpen == nullptr)
	{
		config.Callback.OnOpen = [=](MSRef<IChannel> channel)
		{
			channel->getPipeline()->addFirst("inbound", MSNew<HTTPServerInboundHandler>(this));
			channel->getPipeline()->addLast("outbound", MSNew<HTTPServerOutboundHandler>(this));
		};
	}
	if (config.Callback.OnRoute == nullptr)
	{
		config.Callback.OnRoute = [](MSString const& rule, MSString const& url)-> bool
		{
			try
			{
				std::regex regex(rule);
				return std::regex_match(url, regex);
			}
			catch (...) {}
			return false;
		};
	}
	m_OnRoute = config.Callback.OnRoute;
	m_Reactor = MSNew<TCPServerReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Backlog,
		config.Workers,
		TCPServerReactor::callback_tcp_t{config.Callback.OnOpen, config.Callback.OnClose}
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
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

bool HTTPServer::bind_internal(MSStringView path, uint8_t type, method_t&& method)
{
	switch (type)
	{
	case HTTP_DELETE:
		{
			MSMutexLock lock(m_LockDelete);
			auto result = std::find_if(m_DeleteRouter.begin(), m_DeleteRouter.end(), [=](auto& e)-> bool { return e.first == path; });
			if (result == m_DeleteRouter.end()) m_DeleteRouter.emplace_back(MSString(path), method);
			else result->second = method;
		}
		break;
	case HTTP_GET:
		{
			MSMutexLock lock(m_LockGet);
			auto result = std::find_if(m_GetRouter.begin(), m_GetRouter.end(), [=](auto& e)-> bool { return e.first == path; });
			if (result == m_GetRouter.end()) m_GetRouter.emplace_back(MSString(path), method);
			else result->second = method;
		}
		break;
	case HTTP_POST:
		{
			MSMutexLock lock(m_LockPost);
			auto result = std::find_if(m_PostRouter.begin(), m_PostRouter.end(), [=](auto& e)-> bool { return e.first == path; });
			if (result == m_PostRouter.end()) m_PostRouter.emplace_back(MSString(path), method);
			else result->second = method;
		}
		break;
	case HTTP_PUT:
		{
			MSMutexLock lock(m_LockPut);
			auto result = std::find_if(m_PutRouter.begin(), m_PutRouter.end(), [=](auto& e)-> bool { return e.first == path; });
			if (result == m_PutRouter.end()) m_PutRouter.emplace_back(MSString(path), method);
			else result->second = method;
		}
		break;
	default:
		MS_WARN("unknown method %d", type);
		return false;
	}
	return true;
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
	// Parse request from client

	m_Settings.on_message_begin = [](http_parser* parser)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		handler->m_LastField = {};
		handler->m_Request.Url = {};
		handler->m_Request.Params.clear();
		handler->m_Request.Header.clear();
		handler->m_Request.Body = {};

		handler->m_Response.Code = 0;
		handler->m_Response.Header.clear();
		handler->m_Response.Body = {};
		return 0;
	};
	m_Settings.on_url = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		auto url = MSString(at, length);

		// ex: url = /path/to/resource?param1=value1&param2=value2&...

		auto index = url.find('?');
		if (index != MSString::npos)
		{
			for (size_t i = index + 1, start = i; i < url.size();)
			{
				auto index1 = url.find('&', start);
				MSString param, key, value;
				if (index1 != MSString::npos) param = url.substr(start, index1 - start);
				else param = url.substr(start);

				auto index2 = param.find('=');
				if (index2 != MSString::npos) key = param.substr(0, index2);
				else key = param;
				if (index2 != MSString::npos) value = param.substr(index2 + 1);

				if (key.empty() == false) handler->m_Request.Params[key] = value;

				if (index1 == MSString::npos) break;
				i = index1 + 1;
				start = i;
			}
		}

		handler->m_Request.Url = url.substr(0, index);
		if (handler->m_Request.Url.empty()) handler->m_Request.Url = "/";
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
		auto result = handler->m_Request.Header.emplace(handler->m_LastField, MSString(at, length));
		if (result.second == false) result.first->second += "," + MSString(at, length);
		return 0;
	};
	m_Settings.on_body = [](http_parser* parser, const char* at, size_t length)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;
		handler->m_Request.Body += MSString(at, length);
		return 0;
	};
	m_Settings.on_message_complete = [](http_parser* parser)
	{
		auto handler = (HTTPServerInboundHandler*)parser->data;

		HTTPServer::method_t method;

		switch (parser->method)
		{
		case HTTP_DELETE:
			{
				MSMutexLock lock(handler->m_Server->m_LockDelete);
				for (auto& route : handler->m_Server->m_DeleteRouter)
				{
					if (handler->m_Server->m_OnRoute(route.first, handler->m_Request.Url) == false) continue;
					method = route.second;
				}
			} break;
		case HTTP_GET:
			{
				MSMutexLock lock(handler->m_Server->m_LockGet);
				for (auto& route : handler->m_Server->m_GetRouter)
				{
					if (handler->m_Server->m_OnRoute(route.first, handler->m_Request.Url) == false) continue;
					method = route.second;
				}
			} break;
		case HTTP_POST:
			{
				MSMutexLock lock(handler->m_Server->m_LockPost);
				for (auto& route : handler->m_Server->m_PostRouter)
				{
					if (handler->m_Server->m_OnRoute(route.first, handler->m_Request.Url) == false) continue;
					method = route.second;
				}
			} break;
		case HTTP_PUT:
			{
				MSMutexLock lock(handler->m_Server->m_LockPut);
				for (auto& route : handler->m_Server->m_PutRouter)
				{
					if (handler->m_Server->m_OnRoute(route.first, handler->m_Request.Url) == false) continue;
					method = route.second;
				}
			} break;
		default: break;
		}

		if (method)
		{
			try
			{
				handler->m_Context->attribs()["request"] = &handler->m_Request;
				handler->m_Context->attribs()["response"] = &handler->m_Response;
				method(handler->m_Request, handler->m_Response);
			}
			catch (MSError const& ex)
			{
				cpptrace::logic_error error(ex.what());
				MS_ERROR("%s", error.what());
			}
			catch (...)
			{
				cpptrace::logic_error error("unhandled exception");
				MS_ERROR("%s", error.what());
			}

			if (handler->m_Response.Code == 0)
			{
				handler->m_Response.Code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
				handler->m_Response.Body = "500 Internal Server Error";
			}
		}
		else
		{
			handler->m_Response.Code = HTTP_STATUS_NOT_FOUND;
			handler->m_Response.Body = "404 Not Found";
		}

		return 0;
	};

	m_Context = context;
	auto parsedNum = http_parser_execute(&m_Parser, &m_Settings, event->Message.data(), event->Message.size());
	if (parsedNum == event->Message.size()) return true;
	context->close();
	return false;
}

HTTPServerOutboundHandler::HTTPServerOutboundHandler(MSRaw<HTTPServer> server)
	:
	m_Server(server)
{
}

bool HTTPServerOutboundHandler::channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	auto& requestData = context->attribs()["request"];
	auto& responseData = context->attribs()["response"];
	if(requestData.type() == typeid(HTTPServer::request_t*) && responseData.type() == typeid(HTTPServer::response_t*))
	{
		auto& request = *std::any_cast<HTTPServer::request_t*>(requestData);
		auto& response = *std::any_cast<HTTPServer::response_t*>(responseData);
		
		if (response.Code)
		{
			// Generate response headers

			MSString headers;
			response.Header.emplace("Content-Type", "text/plain; charset=UTF-8");
			response.Header["Content-Length"] = std::to_string(response.Body.size());
			for (auto& header : response.Header) headers += header.first + ":" + header.second + "\r\n";
			auto responseHTML = "HTTP/1.1" " " + std::to_string(response.Code) + "\r\n" + headers + "\r\n" + response.Body;

			// Send response to remote client

			context->write(IChannelEvent::New(responseHTML));
		}
	}
	return false;
}

bool HTTPServerRequestHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	auto& requestData = context->attribs()["request"];
	auto& responseData = context->attribs()["response"];
	if(requestData.type() == typeid(HTTPServer::request_t*) && responseData.type() == typeid(HTTPServer::response_t*))
	{
		auto& request = *std::any_cast<HTTPServer::request_t*>(requestData);
		auto& response = *std::any_cast<HTTPServer::response_t*>(responseData);
		
		try
		{
			return handle(context, request, response);
		}
		catch (MSError const& ex)
		{
			cpptrace::logic_error error(ex.what());
			MS_ERROR("%s", error.what());
		}
		catch (...)
		{
			cpptrace::logic_error error("unhandled exception");
			MS_ERROR("%s", error.what());
		}
	}
    return false;
}

bool HTTPServerResponseHandler::channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	auto& requestData = context->attribs()["request"];
	auto& responseData = context->attribs()["response"];
	if(requestData.type() == typeid(HTTPServer::request_t*) && responseData.type() == typeid(HTTPServer::response_t*))
	{
		auto& request = *std::any_cast<HTTPServer::request_t*>(requestData);
		auto& response = *std::any_cast<HTTPServer::response_t*>(responseData);
		
		try
		{
			return handle(context, request, response);
		}
		catch (MSError const& ex)
		{
			cpptrace::logic_error error(ex.what());
			MS_ERROR("%s", error.what());
		}
		catch (...)
		{
			cpptrace::logic_error error("unhandled exception");
			MS_ERROR("%s", error.what());
		}
	}
    return false;
}