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
#include "Endpoint/IEndpoint.h"
#include "Reactor/Private/ChannelHandler.h"
#include "Reactor/TCP/TCPClientReactor.h"
#include "HTTPProtocol.h"
#include <http_parser.h>

class HTTPClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		TCPClientReactor::callback_tcp_t Callback;
	};

	using request_t = HTTPRequest;
	using response_t = HTTPResponse;

public:
	explicit HTTPClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool call_get(request_t const& request, uint32_t timeout, response_t& response)
	{
		return call_internal(request, HTTP_GET, timeout, response);
	}

	bool call_post(request_t const& request, uint32_t timeout, response_t& response)
	{
		return call_internal(request, HTTP_POST, timeout, response);
	}

	bool call_put(request_t const& request, uint32_t timeout, response_t& response)
	{
		return call_internal(request, HTTP_PUT, timeout, response);
	}

	bool call_delete(request_t const& request, uint32_t timeout, response_t& response)
	{
		return call_internal(request, HTTP_DELETE, timeout, response);
	}

protected:
	bool call_internal(request_t const& request, uint8_t type, uint32_t timeout, response_t& response);

protected:
	friend class HTTPClientInboundHandler;
	MSMutex m_Lock;
	config_t m_Config;
	MSRef<TCPClientReactor> m_Reactor;

	struct invoke_t
	{
		MSLambda<void(response_t&&)> OnResult;
	};
	MSQueue<invoke_t> m_Sessions;
};

class HTTPClientInboundHandler : public ChannelInboundHandler
{
public:
	explicit HTTPClientInboundHandler(MSRaw<HTTPClient> client);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSRaw<HTTPClient> m_Client;
	http_parser m_Parser;
	http_parser_settings m_Settings;
	MSString m_LastField;
	HTTPClient::response_t m_Response;
};
