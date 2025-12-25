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
#include "Endpoint/RPC/RPCProtocol.h"
#include "Reactor/TCP/TCPClientReactor.h"
#include "Utility/Timer.h"
#include "Utility/TraitsUtility.h"
class RPCClientInboundHandler;

/// @brief RPC Client Base
class RPCClientBase : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT16_MAX;
		TCPClientReactor::callback_tcp_t Callback;
	};
	using method_t = MSLambda<bool(MSStringView const& input, MSString& output)>;

public:
	explicit RPCClientBase(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool unbind(MSStringView name);
	bool invoke(uint32_t hash, MSStringView const& input, MSString& output);
	bool bind(MSStringView name, method_t&& method);

	bool call(MSStringView const& name, uint32_t timeout, MSStringView const& input, MSString& output);
	bool async(MSStringView const& name, uint32_t timeout, MSStringView const& input, MSLambda<void(MSString&&)>&& callback);

protected:
	friend class RPCClientInboundHandler;
	const config_t m_Config;
	Timer m_Timer;
	MSMutex m_LockMethod;
	MSMutex m_LockSession;
	MSAtomic<uint32_t> m_Session;
	MSRef<TCPClientReactor> m_Reactor;
	MSMap<uint32_t, method_t> m_Methods;
	MSMap<uint32_t, MSLambda<void(MSStringView const&)>> m_Sessions;
};

/// @brief RPC Client Endpoint
class RPCClient : public RPCClientBase
{
public:
	using RPCClientBase::bind;
	using RPCClientBase::call;
	using RPCClientBase::async;
	using RPCClientBase::RPCClientBase;

	template<class F, std::enable_if_t<!std::is_same_v<typename TTraits<F>::return_data, TTraits<method_t>::return_data> || !std::is_same_v<typename TTraits<F>::argument_datas, TTraits<method_t>::argument_datas>, int> = 0>
	bool bind(MSStringView name, F method)
	{
		return RPCClientBase::bind(name, [method](MSStringView const& input, MSString& output)->bool
		{
			typename TTraits<F>::argument_datas request;
			if constexpr (TTraits<F>::argument_count != 0)
			{
				if (MSTypeC(input, request) == false) return false;
			}
			if constexpr (std::is_void_v<typename TTraits<F>::return_type>)
			{
				std::apply(method, request);
				return true;
			}
			else
			{
				auto response = std::apply(method, request);
				return MSTypeC(response, output);
			}
		});
	}

	/// @brief synchronous call
	/// @tparam T return type
	/// @tparam Args args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @return result and true if sent
	template <class T, class... Args>
	auto call(MSStringView const& name, uint32_t timeout, Args &&... args)
	{
		if constexpr (std::is_void_v<T>)
		{
			MSString request;
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(std::make_tuple(std::forward<Args>(args)...), request) == false) return false;
			}
			MSString response;
			return RPCClientBase::call(name, timeout, request, response);
		}
		else
		{
			MSString request;
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(std::make_tuple(std::forward<Args>(args)...), request) == false) return MSBinary{T{}, false};
			}
			MSString response;
			if (RPCClientBase::call(name, timeout, request, response))
			{
				T result;
				if (MSTypeC(response, result) == true) return MSBinary{result, true};
			}
			return MSBinary{T{}, false};
		}
	}

	/// @brief asynchronous call
	/// @tparam F callback type
	/// @tparam Args args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @param callback call back
	/// @return return true if sent
	template <class F, class... Args>
	bool async(MSStringView const& name, uint32_t timeout, MSTuple<Args...> const& args, F callback)
	{
		MSString request;
		if constexpr (sizeof...(Args) != 0)
		{
			if (MSTypeC(args, request) == false) return false;
		}
		return RPCClientBase::async(name, timeout, request, [callback](MSString&& response)
		{
			typename TTraits<F>::argument_datas result;
			if constexpr (TTraits<F>::argument_count) MSTypeC(response, std::get<0>(result));
			std::apply(callback, result);
		});
	}
};