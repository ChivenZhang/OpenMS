#pragma once
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
#include "OpenMS/Endpoint/IEndpoint.h"
#include "OpenMS/Service/IProperty.h"
#include "OpenMS/Service/Private/Property.h"
#include "OpenMS/Reactor/TCP/TCPServerReactor.h"
#include "OpenMS/Reactor/Private/ChannelHandler.h"

class RemoteServer : public IEndpoint
{
public:
	struct config_t
	{
		TString IP;
		uint16_t PortNum = 0;
		uint32_t Backlog = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT32_MAX;
		TCPServerReactor::callback_tcp_t Callback;
	};

public:
	void startup() override;
	void shutdown() override;
	bool unbind(TStringView name);
	bool invoke(TStringView name, TString const& input, TString& output);
	virtual void configureEndpoint(config_t& config) = 0;

	template<class F, OPENMS_NOT_SAME(typename std::function_traits<F>::result_type, void)>
	bool bind(TStringView name, F method)
	{
		auto callback = [method](TString const& input, TString& output) -> bool {
			// Convert input to tuple

			typename std::function_traits<F>::argument_tuple args;
			if (std::is_same_v<decltype(args), TTuple<>> == false)
			{
				if (TTypeC(input, args) == false) return false;
			}

			// Call method with tuple

			auto result = std::apply(method, args);

			// Convert result to output

			if (TTypeC(result, output) == false) return false;
			return true;
			};

		return bind_internal(name, callback);
	}

	template<class F, OPENMS_IS_SAME(typename std::function_traits<F>::result_type, void)>
	bool bind(TStringView name, F method)
	{
		auto callback = [method](TString const& input, TString& output) -> bool {
			// Convert input to tuple

			typename std::function_traits<F>::argument_tuple args;
			if (std::is_same_v<decltype(args), TTuple<>> == false)
			{
				if (TTypeC(input, args) == false) return false;
			}

			// Call method with tuple

			std::apply(method, args);

			// Return empty output
			return true;
			};

		return bind_internal(name, callback);
	}

protected:
	bool bind_internal(TStringView name, TLambda<bool(TString const&, TString&)> method);

protected:
	friend class RemoteServerInboundHandler;
	TMutex m_Lock;
	uint32_t m_Buffers = UINT32_MAX;	// bytes from property
	TRef<TCPServerReactor> m_Reactor;
	TMap<TString, TLambda<bool(TString const& input, TString& output)>> m_Methods;
};

class RemoteServerInboundHandler : public ChannelInboundHandler
{
public:
	RemoteServerInboundHandler(TRaw<RemoteServer> server);
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) override;

protected:
	TString m_Buffer;
	TRaw<RemoteServer> m_Server;
};