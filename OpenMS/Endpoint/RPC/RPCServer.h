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
#include "OpenMS/Service/Private/Property.h"
#include "OpenMS/Endpoint/RPC/RPCProtocol.h"
#include "OpenMS/Reactor/TCP/TCPServerReactor.h"
#include "OpenMS/Reactor/Private/ChannelHandler.h"

class RPCServer : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Backlog = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT16_MAX;
		TCPServerReactor::callback_tcp_t Callback;
	};

public:
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;
	bool unbind(MSStringView name);
	bool invoke(MSStringView name, MSString const& input, MSString& output);

	template <class F, OPENMS_NOT_SAME(typename std::function_traits<F>::result_type, void)>
	bool bind(MSStringView name, F method)
	{
		auto callback = [method](MSString const& input, MSString& output)-> bool
		{
			// Convert input to tuple

			typename std::function_traits<F>::argument_tuple args;
			if (std::is_same_v<decltype(args), MSTuple<>> == false)
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

	template <class F, OPENMS_IS_SAME(typename std::function_traits<F>::result_type, void)>
	bool bind(MSStringView name, F method)
	{
		auto callback = [method](MSString const& input, MSString& output) -> bool
		{
			// Convert input to tuple

			typename std::function_traits<F>::argument_tuple args;
			if (std::is_same_v<decltype(args), MSTuple<>> == false)
			{
				if (TTypeC(input, args) == false) return false;
			}

			// Call method with tuple

			std::apply(method, args);

			// Return void output
			return true;
		};

		return bind_internal(name, callback);
	}

protected:
	virtual void configureEndpoint(config_t& config) = 0;

	bool bind_internal(MSStringView name, MSLambda<bool(MSString const&, MSString&)> method);

protected:
	friend class RPCServerInboundHandler;
	MSMutex m_Lock;
	uint32_t m_Buffers = UINT32_MAX;
	MSRef<TCPServerReactor> m_Reactor;
	MSStringMap<MSLambda<bool(MSString const& input, MSString& output)>> m_Methods;
};

class RPCServerInboundHandler : public ChannelInboundHandler
{
public:
	explicit RPCServerInboundHandler(MSRaw<RPCServer> server);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<RPCServer> m_Server;
};