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
#include "Service/Private/Property.h"
#include "Endpoint/RPC/RPCProtocol.h"
#include "Reactor/TCP/TCPServerReactor.h"
#include "Reactor/Private/ChannelHandler.h"

class RPCServerInboundHandler;

/// @brief RPC Server Endpoint
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
	};

public:
	explicit RPCServer(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;
	bool unbind(MSStringView name);
	bool invoke(MSStringView name, MSString const& input, MSString& output);

	template <class F, OPENMS_NOT_SAME(typename MSTraits<F>::return_type, void)>
	bool bind(MSStringView name, F method)
	{
		auto callback = [method](MSString const& input, MSString& output)-> bool
		{
			// Convert request to tuple

			typename MSTraits<F>::argument_types args;
			if (std::is_same_v<decltype(args), MSTuple<>> == false)
			{
				if (MSTypeC(input, args) == false) return false;
			}

			// Call method with tuple args

			auto result = std::apply(method, args);

			// Convert request to string

			if (MSTypeC(result, output) == false) return false;
			return true;
		};

		return bind_internal(name, callback);
	}

	template <class F, OPENMS_IS_SAME(typename MSTraits<F>::return_type, void)>
	bool bind(MSStringView name, F method)
	{
		auto callback = [method](MSString const& input, MSString& output) -> bool
		{
			// Convert request to tuple

			typename MSTraits<F>::argument_types args;
			if (std::is_same_v<decltype(args), MSTuple<>> == false)
			{
				if (MSTypeC(input, args) == false) return false;
			}

			// Call method with tuple args

			std::apply(method, args);

			// Return void output
			return true;
		};

		return bind_internal(name, callback);
	}

protected:
	bool bind_internal(MSStringView name, MSLambda<bool(MSString const&, MSString&)> method);

protected:
	friend class RPCServerInboundHandler;
	const config_t m_Config;
	MSMutex m_Locker;
	MSRef<TCPServerReactor> m_Reactor;
	MSStringMap<MSLambda<bool(MSString const& input, MSString& output)>> m_Methods;
};