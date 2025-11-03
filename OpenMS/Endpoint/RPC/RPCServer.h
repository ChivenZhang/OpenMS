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
#include "OpenMS/Server/Private/Property.h"
#include "Endpoint/RPC/RPCProtocol.h"
#include "Reactor/TCP/TCPServerReactor.h"
#include "Utility/Timer.h"
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
	};
	using method_t = MSLambda<bool(MSStringView const& input, MSString& output)>;

public:
	explicit RPCServerBase(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool unbind(MSStringView name);
	bool invoke(uint32_t hash, MSStringView const& input, MSString& output);
	bool bind(MSStringView name, MSLambda<bool(MSStringView const& input, MSString& output)>&& method);

	MSBinary<MSString, bool> call(MSStringView const& name, uint32_t timeout, MSStringView const& input);
	bool async(MSStringView const& name, uint32_t timeout, MSStringView const& input, MSLambda<void(MSString&&)>&& callback);

protected:
	friend class RPCServerInboundHandler;
	const config_t m_Config;
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

	template <class F, OPENMS_NOT_SAME(typename MSTraits<F>::return_type, void)>
	bool bind(MSStringView name, F method)
	{
		return RPCServerBase::bind(name, [method](MSStringView const& input, MSString& output)-> bool
		{
			// Convert request to tuple

			typename MSTraits<F>::argument_datas args;
			if (std::is_same_v<decltype(args), MSTuple<>> == false)
			{
				if (MSTypeC(input, args) == false) return false;
			}

			// Call method with tuple args

			auto result = std::apply(method, args);

			// Convert request to string

			if (MSTypeC(result, output) == false) return false;
			return true;
		});
	}

	template <class F, OPENMS_IS_SAME(typename MSTraits<F>::return_type, void)>
	bool bind(MSStringView name, F method)
	{
		return RPCServerBase::bind(name, [method](MSStringView const& input, MSString& output) -> bool
		{
			// Convert request to tuple

			typename MSTraits<F>::argument_datas args;
			if (std::is_same_v<decltype(args), MSTuple<>> == false)
			{
				if (MSTypeC(input, args) == false) return false;
			}

			// Call method with tuple args

			std::apply(method, args);

			// Return void output
			return true;
		});
	}

	/// @brief synchronous call
	/// @tparam T return type
	/// @tparam Args args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @return result and true if sent
	template <class T, class... Args, OPENMS_NOT_SAME(T, void)>
	MSBinary<T, bool> call(MSStringView const& name, uint32_t timeout, Args &&... args)
	{
		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (MSTypeC(std::make_tuple(std::forward<Args>(args)...), input) == false) return {T(), false};
		}

		auto output = RPCServerBase::call(name, timeout, input);

		// Convert string to response

		if (output.second)
		{
			T response;
			if (MSTypeC(output.first, response) == true) return {response, true};
		}
		return {{}, false};
	}

	/// @brief synchronous call and return void
	/// @tparam T void
	/// @tparam Args args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @return return true if sent
	template <class T, class... Args, OPENMS_IS_SAME(T, void)>
	bool call(MSStringView const& name, uint32_t timeout, Args &&... args)
	{
		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (MSTypeC(std::make_tuple(std::forward<Args>(args)...), input) == false) return false;
		}

		auto output = RPCServerBase::call(name, timeout, input);
		return output.second;
	}

	/// @brief asynchronous call
	/// @tparam T return type
	/// @tparam Args args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @param callback call back
	/// @return return true if sent
	template <class T, class... Args, OPENMS_NOT_SAME(T, void)>
	bool async(MSStringView const& name, uint32_t timeout, MSTuple<Args...> const& args, MSLambda<void(T&&)> callback)
	{
		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (MSTypeC(args, input) == false) return false;
		}

		// Convert string to response

		return RPCServerBase::async(name, timeout, input, [callback](MSString&& output)
		{
			T response;
			MSTypeC(output, response);
			if (callback) callback(std::move(response));
		});
	}

	/// @brief asynchronous call and return void
	/// @tparam T void
	/// @tparam Args args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @param callback call back
	/// @return return true if sent
	template <class T, class... Args, OPENMS_IS_SAME(T, void)>
	bool async(MSStringView const& name, uint32_t timeout, MSTuple<Args...> const& args, MSLambda<void()> callback)
	{
		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (MSTypeC(args, input) == false) return false;
		}

		// Convert string to response

		return RPCServerBase::async(name, timeout, input, [callback](MSString&& output)
		{
			if (callback) callback();
		});
	}
};