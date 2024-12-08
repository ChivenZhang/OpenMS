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
#include "OpenMS/Toolkit/Timer.h"
#include "OpenMS/Service/IProperty.h"
#include "OpenMS/Service/Private/Property.h"
#include "OpenMS/Reactor/TCP/TCPClientReactor.h"
#include "OpenMS/Reactor/Private/ChannelHandler.h"

struct RPCClientRequest
{
	uint32_t indx;
	MSString name;
	MSString args;
	OPENMS_TYPE(RPCClientRequest, indx, name, args)
};

struct RPCClientResponse
{
	uint32_t indx;
	MSString args;
	OPENMS_TYPE(RPCClientResponse, indx, args)
};

class RPCClient : public IEndpoint
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

	struct invoke_t
	{
		MSLambda<void(MSString&&)> OnResult;
	};

public:
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;
	virtual void configureEndpoint(config_t& config) = 0;

	template<class T, class... Args, OPENMS_NOT_SAME(T, void)>
	T call(MSStringView name, uint32_t timeout/*ms*/, Args... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return T();

		// Convert arguments to string

		MSString input, output;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (TTypeC(std::make_tuple(args...), input) == false) return T();
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return T();

		// Set up promise and callback

		auto promise = MSNew<MSPromise<void>>();
		auto future = promise->get_future();
		{
			MSMutexLock lock(m_Lock);
			auto& package = m_Packages[request.indx];
			package.OnResult = [promise, &output](MSString&& response) {
				output = response;
				promise->set_value();
				};
		}

		// Send input to remote server

		auto event = MSNew<IChannelEvent>();
		event->Message = input + char(); // Use '\0' to split the message
		m_Reactor->writeAndFlush(event, nullptr);

		// Wait for result and return

		auto status = future.wait_for(std::chrono::milliseconds(timeout));
		{
			MSMutexLock lock(m_Lock);
			m_Packages.erase(request.indx);
		}
		if (status == std::future_status::ready)
		{
			T result;
			if (TTypeC(output, result) == false) return T();
			return result;
		}
		return T();
	}

	template<class T, class... Args, OPENMS_IS_SAME(T, void)>
	T call(MSStringView name, uint32_t timeout/*ms*/, Args... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return T();

		// Convert arguments to string

		MSString input;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (TTypeC(std::make_tuple(args...), input) == false) return T();
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return T();

		// Set up promise and callback

		auto promise = MSNew<MSPromise<void>>();
		auto future = promise->get_future();
		{
			MSMutexLock lock(m_Lock);
			auto& package = m_Packages[request.indx];
			package.OnResult = [promise](MSString&& response) { promise->set_value(); };
		}

		// Send input to remote server

		auto event = MSNew<IChannelEvent>();
		event->Message = input + char(); // Use '\0' to split the message
		m_Reactor->writeAndFlush(event, nullptr);

		// Wait for result and return

		future.wait_for(std::chrono::milliseconds(timeout));
		{
			MSMutexLock lock(m_Lock);
			m_Packages.erase(request.indx);
		}
	}

	template<class T, class... Args, OPENMS_NOT_SAME(T, void)>
	bool async(MSStringView name, uint32_t timeout/*ms*/, MSTuple<Args...> args, MSLambda<void(T&&)> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert arguments to string

		MSString input;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (TTypeC(args, input) == false) return false;
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return false;

		// Set up promise and callback

		{
			MSMutexLock lock(m_Lock);
			auto& package = m_Packages[request.indx];
			package.OnResult = [callback](MSString&& response) {
				T result;
				TTypeC(response, result);
				if (callback) callback(std::move(result));
				};
		}

		m_Timer.start(timeout, 0, [=, packageID = request.indx](uint32_t handle) {
			MSMutexLock lock(m_Lock);
			auto result = m_Packages.find(packageID);
			if (result != m_Packages.end())
			{
				result->second.OnResult(MSString());
				m_Packages.erase(result);
			}
			});


		// Send input to remote server

		auto event = MSNew<IChannelEvent>();
		event->Message = input + char(); // Use '\0' to split the message
		m_Reactor->writeAndFlush(event, nullptr);
		return true;
	}

	template<class T, class... Args, OPENMS_IS_SAME(T, void)>
	bool async(MSStringView name, uint32_t timeout/*ms*/, MSTuple<Args...> args, MSLambda<void()> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert arguments to string

		MSString input;
		if (std::is_same_v<MSTuple<Args...>, MSTuple<>> == false)
		{
			if (TTypeC(args, input) == false) return false;
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return false;

		// Set up promise and callback

		{
			MSMutexLock lock(m_Lock);
			auto& package = m_Packages[request.indx];
			package.OnResult = [callback](MSString&& response) {
				if (callback) callback();
				};
		}

		m_Timer.start(timeout, 0, [=, packageID = request.indx](uint32_t handle) {
			MSMutexLock lock(m_Lock);
			auto result = m_Packages.find(packageID);
			if (result != m_Packages.end())
			{
				result->second.OnResult(MSString());
				m_Packages.erase(result);
			}
			});

		// Send input to remote server

		auto event = MSNew<IChannelEvent>();
		event->Message = input + char(); // Use '\0' to split the message
		m_Reactor->writeAndFlush(event, nullptr);
		return true;
	}

protected:
	friend class RPCClientInboundHandler;
	uint32_t m_PackageID = 0;
	uint32_t m_Buffers = UINT32_MAX;	// bytes from property
	MSMutex m_Lock;
	Timer m_Timer;
	MSRef<TCPClientReactor> m_Reactor;
	MSMap<uint32_t, invoke_t> m_Packages;
};

class RPCClientInboundHandler : public ChannelInboundHandler
{
public:
	RPCClientInboundHandler(MSRaw<RPCClient> client);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<RPCClient> m_Client;
};