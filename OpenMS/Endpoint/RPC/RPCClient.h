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
#include "Reactor/TCP/TCPClientReactor.h"
#include "Utility/Timer.h"
class RPCClientInboundHandler;

/// @brief RPC Client Endpoint
class IRPCClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT16_MAX;
	};

public:
	explicit IRPCClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;
	MSBinary<MSString, bool> call(MSStringView const& name, uint32_t timeout, MSString const& input);
	bool async(MSStringView const& name, uint32_t timeout, MSString const& input, MSLambda<void(MSString&&)>&& callback);

protected:
	friend class RPCClientInboundHandler;
	const config_t m_Config;
	Timer m_Timer;
	MSMutex m_Locker;
	MSAtomic<uint32_t> m_Session;
	MSRef<TCPClientReactor> m_Reactor;
	MSMap<uint32_t, MSLambda<void(MSStringView const&)>> m_Sessions;
};

template<class Parser = RPCProtocolParser>
class TRPCClient : public IRPCClient
{
public:
	using parser_t = Parser;
	using IRPCClient::IRPCClient;

	/// @brief synchronous call
	/// @tparam T return type
	/// @tparam ARGS args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @return result and true if sent
	template <class T, class... ARGS, OPENMS_NOT_SAME(T, void)>
	MSBinary<T, bool> call(MSStringView const& name, uint32_t timeout, ARGS &&... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return {T(), false};

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (parser_t::toString(std::make_tuple(std::forward<ARGS>(args)...), input) == false) return {T(), false};
		}

		auto response = IRPCClient::call(name, timeout, input);

		// Convert string to response

		if (response.second)
		{
			T result;
			if (parser_t::fromString(response.first, result) == true) return {result, true};
		}
		return {T(), false};
	}

	/// @brief synchronous call and return void
	/// @tparam T void
	/// @tparam ARGS args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @return return true if sent
	template <class T, class... ARGS, OPENMS_IS_SAME(T, void)>
	bool call(MSStringView const& name, uint32_t timeout, ARGS &&... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (parser_t::toString(std::make_tuple(std::forward<ARGS>(args)...), input) == false) return false;
		}

		auto response = IRPCClient::call(name, timeout, std::move(input));
		return response.second;
	}

	/// @brief asynchronous call
	/// @tparam T return type
	/// @tparam ARGS args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @param callback call back
	/// @return return true if sent
	template <class T, class... ARGS, OPENMS_NOT_SAME(T, void)>
	bool async(MSStringView const& name, uint32_t timeout, MSTuple<ARGS...> const& args, MSLambda<void(T&&)> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (parser_t::toString(args, input) == false) return false;
		}

		// Convert string to response

		return IRPCClient::async(name, timeout, input, [callback](MSString&& response)
		{
			T result;
			parser_t::fromString(response, result);
			if (callback) callback(std::move(result));
		});
	}

	/// @brief asynchronous call and return void
	/// @tparam T void
	/// @tparam ARGS args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @param callback call back
	/// @return return true if sent
	template <class T, class... ARGS, OPENMS_IS_SAME(T, void)>
	bool async(MSStringView const& name, uint32_t timeout, MSTuple<ARGS...> const& args, MSLambda<void()> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (parser_t::toString(args, input) == false) return false;
		}

		// Convert string to response

		return IRPCClient::async(name, timeout, input, [callback](MSString&& response)
		{
			if (callback) callback();
		});
	}
};

using RPCClient = TRPCClient<>;