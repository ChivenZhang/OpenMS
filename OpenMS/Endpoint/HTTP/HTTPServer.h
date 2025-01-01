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
#include "OpenMS/Reactor/TCP/TCPServerReactor.h"
#include "HTTPProtocol.h"
#include <http_parser.h>

class HTTPServer : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Backlog = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT32_MAX;
		TCPServerReactor::callback_tcp_t Callback;
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

	bool bind_get(MSString path, callback_t callback);

	bool bind_post(MSString path, callback_t callback);

	bool bind_put(MSString path, callback_t callback);

	bool bind_delete(MSString path, callback_t callback);

protected:
	virtual void configureEndpoint(config_t& config) const = 0;

protected:
	friend class HTTPServerInboundHandler;
	uint32_t m_Buffers = UINT32_MAX;
	MSRef<TCPServerReactor> m_Reactor;
	MSMutex m_LockGet, m_LockPost, m_LockPut, m_LockDelete;

	struct method_t
	{
		uint8_t Method;
		callback_t Callback;
	};
	MSStringMap<method_t> m_GetMethods, m_PostMethods, m_PutMethods, m_DeleteMethods;
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