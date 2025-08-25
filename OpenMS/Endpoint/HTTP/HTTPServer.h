#pragma once
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
#include "OpenMS/Endpoint/IEndpoint.h"
#include "Reactor/Private/ChannelHandler.h"
#include "OpenMS/Reactor/TCP/TCPServerReactor.h"
#include "HTTPProtocol.h"
#include <http_parser.h>

class HTTPServer : public IEndpoint
{
public:
	using request_t = HTTPRequest;
	using response_t = HTTPResponse;
	using method_t = MSLambda<void(request_t const& request, response_t& response)>;

	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Backlog = 0;
		uint32_t Workers = 0;
		struct
		{
			MSLambda<void(MSRef<IChannel>)> OnOpen;
			MSLambda<void(MSRef<IChannel>)> OnClose;
			MSLambda<bool(MSString const& rule, MSString const& url)> OnRoute;
			MSLambda<void(request_t const& request, response_t& response)> OnError;
		} Callback;
	};
public:
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool bind_get(MSStringView path, method_t&& method)
	{
		return bind_internal(path, HTTP_GET, std::move(method));
	}

	bool bind_post(MSStringView path, method_t&& method)
	{
		return bind_internal(path, HTTP_POST, std::move(method));
	}

	bool bind_put(MSStringView path, method_t&& method)
	{
		return bind_internal(path, HTTP_PUT, std::move(method));
	}

	bool bind_delete(MSStringView path, method_t&& method)
	{
		return bind_internal(path, HTTP_DELETE, std::move(method));
	}

protected:
	bool bind_internal(MSStringView path, uint8_t type, method_t&& method);

	virtual void configureEndpoint(config_t& config) = 0;

protected:
	friend class HTTPServerInboundHandler;
	MSRef<TCPServerReactor> m_Reactor;
	MSMutex m_LockGet, m_LockPost, m_LockPut, m_LockDelete;
	MSLambda<bool(MSString const& rule, MSString const& url)> m_OnRoute;
	MSLambda<void(request_t const& request, response_t& response)> m_OnError;
	MSList<MSBinary<MSString, method_t>> m_GetRouter, m_PostRouter, m_PutRouter, m_DeleteRouter;
};

class HTTPServerInboundHandler : public ChannelInboundHandler
{
public:
	explicit HTTPServerInboundHandler(MSRaw<HTTPServer> server);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSRaw<HTTPServer> m_Server;
	http_parser m_Parser;
	http_parser_settings m_Settings;
	MSString m_LastField;
	HTTPServer::request_t m_Request;
	HTTPServer::response_t m_Response;
};
