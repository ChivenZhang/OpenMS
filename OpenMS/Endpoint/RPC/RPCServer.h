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
#include "Reactor/TCP/TCPServerReactor.h"
#include "Utility/Timer.h"
#include "Utility/TraitsUtility.h"
class RPCServerInboundHandler;

/// @brief RPC Server Base
class RPCServerBase : public IEndpoint
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
	using method_t = MSLambda<bool(MSHnd<IChannel> client, MSStringView const& input, MSString& output)>;

public:
	explicit RPCServerBase(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool unbind(MSStringView name);
	bool invoke(MSHnd<IChannel> client, uint32_t hash, MSStringView const& input, MSString& output);
	bool bind(MSStringView name, method_t&& method);

	bool call(MSHnd<IChannel> client, MSStringView const& name, uint32_t timeout, MSStringView const& input, MSString& output);
	bool async(MSHnd<IChannel> client, MSStringView const& name, uint32_t timeout, MSStringView const& input, MSLambda<void(MSString&&)>&& callback);

protected:
	friend class RPCServerInboundHandler;
	config_t m_Config;
	Timer m_Timer;
	MSMutex m_LockMethod;
	MSMutex m_LockSession;
	MSAtomic<uint32_t> m_Session;
	MSRef<TCPServerReactor> m_Reactor;
	MSMap<uint32_t, method_t> m_Methods;
	MSMap<uint32_t, MSLambda<void(MSStringView const&)>> m_Sessions;
};

/// @brief RPC Server Endpoint
class RPCServer : public RPCServerBase
{
public:
	using RPCServerBase::bind;
	using RPCServerBase::call;
	using RPCServerBase::async;
	using RPCServerBase::RPCServerBase;

	template<class F, std::enable_if_t<!std::is_same_v<typename TTraits<F>::return_data, TTraits<method_t>::return_data> || !std::is_same_v<typename TTraits<F>::argument_datas, TTraits<method_t>::argument_datas>, int> = 0>
	bool bind(MSStringView name, F method)
	{
		return RPCServerBase::bind(name, [method](MSHnd<IChannel> client, MSStringView const& input, MSString& output)->bool
		{
			typename TSecondTypes<typename TTraits<F>::argument_datas>::second_datas request;
			if constexpr (TSecondTypes<typename TTraits<F>::argument_datas>::second_count != 0)
			{
				if (MSTypeC(input, request) == false) return false;
			}
			if constexpr (std::is_void_v<typename TTraits<F>::return_type>)
			{
				std::apply([&](auto&&... args)
				{
					method(client, std::forward<decltype(args)>(args)...);
				}, request);
				return true;
			}
			else
			{
				auto response = std::apply([&](auto&&... args)
				{
					return method(client, std::forward<decltype(args)>(args)...);
				}, request);
				return MSTypeC(response, output);
			}
		});
	}

	/// @brief synchronous call
	/// @param client client
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @return result and true if sent
	template <class T, class... Args>
	auto call(MSHnd<IChannel> client, MSStringView const& name, uint32_t timeout, Args &&... args)
	{
		if constexpr (std::is_void_v<T>)
		{
			MSString request;
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(std::make_tuple(std::forward<Args>(args)...), request) == false) return false;
			}
			MSString response;
			return RPCServerBase::call(client, name, timeout, request, response);
		}
		else
		{
			MSString request;
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(std::make_tuple(std::forward<Args>(args)...), request) == false) return MSBinary{T{}, false};
			}
			MSString response;
			if (RPCServerBase::call(client, name, timeout, request, response))
			{
				T result;
				if (MSTypeC(response, result) == true) return MSBinary{result, true};
			}
			return MSBinary{T{}, false};
		}
	}

	/// @brief asynchronous call
	/// @param client client
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @param callback call back
	/// @return return true if sent
	template <class F, class... Args>
	bool async(MSHnd<IChannel> client, MSStringView const& name, uint32_t timeout, MSTuple<Args...> const& args, F callback)
	{
		MSString request;
		if constexpr (sizeof...(Args) != 0)
		{
			if (MSTypeC(args, request) == false) return false;
		}
		return RPCServerBase::async(client, name, timeout, request, [callback](MSString&& response)
		{
			typename TTraits<F>::argument_datas result;
			if constexpr (TTraits<F>::argument_count) MSTypeC(response, std::get<0>(result));
			std::apply(callback, result);
		});
	}
};