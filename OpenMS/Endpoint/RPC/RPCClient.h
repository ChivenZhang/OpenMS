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
class RPCClient : public IEndpoint
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
	explicit RPCClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	/// @brief synchronous call
	/// @tparam T return type
	/// @tparam ARGS args types
	/// @param name call name
	/// @param timeout unit: ms
	/// @param args call args
	/// @return result and true if sent
	template <class T, class... ARGS, OPENMS_NOT_SAME(T, void)>
	MSBinary<T, bool> call(MSStringView name, uint32_t timeout, ARGS &&... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return {T(), false};

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (MSTypeC(std::make_tuple(std::forward<ARGS>(args)...), input) == false) return {T(), false};
		}

		MSString output(sizeof(RPCRequestView) + input.size(), 0);
		auto& requestView = *(RPCRequestView*)output.data();
		requestView.ID = ++m_Session;
		requestView.Name = MSHash(name);
		requestView.Length = (uint32_t)input.size();
		if (requestView.Length) ::memcpy(requestView.Buffer, input.data(), input.size());

		// Set promise to handle response

		MSPromise<MSString> promise;
		auto future = promise.get_future();
		{
			MSMutexLock lock(m_Locker);
			auto& session = m_Sessions[requestView.ID];
			session = [&](MSStringView const& response) { promise.set_value(MSString(response)); };
		}

		// Send request to remote server

		auto length = (uint32_t)output.size();
		m_Reactor->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output), nullptr);
		auto status = future.wait_for(std::chrono::milliseconds(timeout));
		{
			MSMutexLock lock(m_Locker);
			m_Sessions.erase(requestView.ID);
		}
		if (status == std::future_status::ready)
		{
			T result;
			if (MSTypeC(future.get(), result) == true) return {result, true};
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
	bool call(MSStringView name, uint32_t timeout, ARGS &&... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (MSTypeC(std::make_tuple(std::forward<ARGS>(args)...), input) == false) return false;
		}

		MSString output(sizeof(RPCRequestView) + input.size(), 0);
		auto& requestView = *(RPCRequestView*)output.data();
		requestView.ID = ++m_Session;
		requestView.Name = MSHash(name);
		requestView.Length = (uint32_t)input.size();
		if (requestView.Length) ::memcpy(requestView.Buffer, input.data(), input.size());

		// Set promise to handle response

		MSPromise<void> promise;
		auto future = promise.get_future();
		{
			MSMutexLock lock(m_Locker);
			auto& session = m_Sessions[requestView.ID];
			session = [&](MSStringView const& response) { promise.set_value(); };
		}

		// Send request to remote server

		auto length = (uint32_t)output.size();
		m_Reactor->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output), nullptr);
		future.wait_for(std::chrono::milliseconds(timeout));
		{
			MSMutexLock lock(m_Locker);
			m_Sessions.erase(requestView.ID);
		}
		return true;
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
	bool async(MSStringView name, uint32_t timeout, MSTuple<ARGS...> const& args, MSLambda<void(T&&)> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (MSTypeC(args, input) == false) return false;
		}

		MSString output(sizeof(RPCRequestView) + input.size(), 0);
		auto& requestView = *(RPCRequestView*)output.data();
		requestView.ID = ++m_Session;
		requestView.Name = MSHash(name);
		requestView.Length = (uint32_t)input.size();
		if (requestView.Length) ::memcpy(requestView.Buffer, input.data(), input.size());

		// Set timer to handle response

		{
			MSMutexLock lock(m_Locker);
			auto& session = m_Sessions[requestView.ID];
			session = [callback](MSStringView const& response)
			{
				T result;
				MSTypeC(response, result);
				if (callback) callback(std::move(result));
			};
		}

		m_Timer.start(timeout, 0, [sessionID = requestView.ID, this](uint32_t handle)
		{
			decltype(m_Sessions)::value_type::second_type callback;
			{
				MSMutexLock lock(m_Locker);
				auto result = m_Sessions.find(sessionID);
				if (result != m_Sessions.end())
				{
					callback = result->second;
					m_Sessions.erase(result);
				}
			}
			if (callback) callback({});
		});

		// Send request to remote server

		uint32_t length = (uint32_t)output.size();
		m_Reactor->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output), nullptr);
		return true;
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
	bool async(MSStringView name, uint32_t timeout, MSTuple<ARGS...> const& args, MSLambda<void()> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert request to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (MSTypeC(args, input) == false) return false;
		}

		MSString output(sizeof(RPCRequestView) + input.size(), 0);
		auto& requestView = *(RPCRequestView*)output.data();
		requestView.ID = ++m_Session;
		requestView.Name = MSHash(name);
		requestView.Length = (uint32_t)input.size();
		if (requestView.Length) ::memcpy(requestView.Buffer, input.data(), input.size());

		// Set timer to handle response

		{
			MSMutexLock lock(m_Locker);
			auto& session = m_Sessions[requestView.ID];
			session = [callback](MSStringView const& response) { if (callback) callback(); };
		}

		m_Timer.start(timeout, 0, [sessionID = requestView.ID, this](uint32_t handle)
		{
			decltype(m_Sessions)::value_type::second_type callback;
			{
				MSMutexLock lock(m_Locker);
				auto result = m_Sessions.find(sessionID);
				if (result != m_Sessions.end())
				{
					callback = result->second;
					m_Sessions.erase(result);
				}
			}
			if (callback) callback({});
		});

		// Send request to remote server

		auto length = (uint32_t)output.size();
		m_Reactor->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output), nullptr);
		return true;
	}

protected:
	friend class RPCClientInboundHandler;
	const config_t m_Config;
	Timer m_Timer;
	MSMutex m_Locker;
	MSAtomic<uint32_t> m_Session;
	MSRef<TCPClientReactor> m_Reactor;
	MSMap<uint32_t, MSLambda<void(MSStringView const&)>> m_Sessions;
};