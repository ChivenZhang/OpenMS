#pragma once
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
#include "OpenMS/Endpoint/IEndpoint.h"
#include "Reactor/Private/ChannelHandler.h"
#include "OpenMS/Reactor/TCP/TCPClientReactor.h"
#include "HTTPProtocol.h"
#include <http_parser.h>

class HTTPClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT32_MAX;
		TCPClientReactor::callback_tcp_t Callback;
	};

	using request_t = HTTPRequest;
	using response_t = HTTPResponse;
	using callback_t = MSLambda<void(request_t const& request, response_t& response)>;

public:
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool call_get(request_t const& request, uint32_t timeout, response_t& response)
	{
		return false;
	}

	bool call_post(request_t const& request, uint32_t timeout, response_t& response)
	{
		return false;
	}

	bool call_put(request_t const& request, uint32_t timeout, response_t& response)
	{
		return false;
	}

	bool call_delete(request_t const& request, uint32_t timeout, response_t& response)
	{
		return false;
	}

	bool async_get(request_t const& request, uint32_t timeout, MSLambda<void(response_t& response)> callback)
	{
		return false;
	}

	bool async_post(request_t const& request, uint32_t timeout, MSLambda<void(response_t& response)> callback)
	{
		return false;
	}

	bool async_put(request_t const& request, uint32_t timeout, MSLambda<void(response_t& response)> callback)
	{
		return false;
	}

	bool async_delete(request_t const& request, uint32_t timeout, MSLambda<void(response_t& response)> callback)
	{
		return false;
	}

protected:
	virtual void configureEndpoint(config_t& config) const = 0;

protected:
	uint32_t m_Buffers = UINT32_MAX;
	MSRef<TCPClientReactor> m_Reactor;
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
	HTTPClient::request_t m_Request;
	HTTPClient::response_t m_Response;
};
