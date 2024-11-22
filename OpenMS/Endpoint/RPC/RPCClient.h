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
#include "OpenMS/Reactor/TCP/TCPClientReactor.h"
#include "OpenMS/Reactor/Private/ChannelHandler.h"

struct RPCClientRequest
{
	uint32_t indx;
	TString name;
	TString args;
	OPENMS_TYPE(RPCClientRequest, indx, name, args)
};

struct RPCClientResponse
{
	uint32_t indx;
	TString args;
	OPENMS_TYPE(RPCClientResponse, indx, args)
};

class RPCClient : public IEndpoint
{
public:
	struct config_t
	{
		TString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT32_MAX;
		TCPClientReactor::callback_tcp_t Callback;
	};

	struct invoke_t
	{
		TLambda<void(TString&&)> OnResult;
	};

public:
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	THnd<IChannelAddress> address() const override;
	virtual void configureEndpoint(config_t& config) = 0;

	template<class T, class... Args, OPENMS_NOT_SAME(T, void)>
	T call(TStringView name, Args... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return T();

		// Convert arguments to string

		TString input, output;
		if (std::is_same_v<TTuple<Args...>, TTuple<>> == false)
		{
			if (TTypeC(std::make_tuple(args...), input) == false) return T();
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return T();

		// Set up promise and callback

		auto promise = TNew<TPromise<bool>>();
		auto future = promise->get_future();
		auto& package = m_Packages[request.indx];
		package.OnResult = [promise, &output](TString&& response) {
			output = response;
			promise->set_value(true);
			};

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		// Use '\0' to split the message
		event->Message = input + char();
		m_Reactor->writeAndFlush(event, nullptr);

		// Wait for result and return

		auto status = future.wait_for(std::chrono::milliseconds(m_ReadTimeout));
		{
			TMutexLock lock(m_Lock);
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
	T call(TStringView name, Args... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return T();

		// Convert arguments to string

		TString input;
		if (std::is_same_v<TTuple<Args...>, TTuple<>> == false)
		{
			if (TTypeC(std::make_tuple(args...), input) == false) return T();
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return T();

		// Set up promise and callback

		auto promise = TNew<TPromise<bool>>();
		auto future = promise->get_future();
		auto& package = m_Packages[request.indx];
		package.OnResult = [promise](TString&& response) {
			promise->set_value(true);
			};

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		// Use '\0' to split the message
		event->Message = input + char();
		m_Reactor->writeAndFlush(event, nullptr);

		// Wait for result and return

		future.wait_for(std::chrono::milliseconds(m_ReadTimeout));
		{
			TMutexLock lock(m_Lock);
			m_Packages.erase(request.indx);
		}
	}

	template<class T, class... Args, OPENMS_NOT_SAME(T, void)>
	bool async(TStringView name, TTuple<Args...> args, TLambda<void(T&&)> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert arguments to string

		TString input;
		if (std::is_same_v<TTuple<Args...>, TTuple<>> == false)
		{
			if (TTypeC(args, input) == false) return false;
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return false;

		// Set up promise and callback

		auto& package = m_Packages[request.indx];
		package.OnResult = [callback](TString&& response) {
			if (callback == nullptr) return;
			T result;
			TTypeC(response, result);
			callback(std::move(result));
			};

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		// Use '\0' to split the message
		event->Message = input + char();
		m_Reactor->writeAndFlush(event, nullptr);
		return true;
	}

	template<class T, class... Args, OPENMS_IS_SAME(T, void)>
	bool async(TStringView name, TTuple<Args...> args, TLambda<void()> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert arguments to string

		TString input;
		if (std::is_same_v<TTuple<Args...>, TTuple<>> == false)
		{
			if (TTypeC(args, input) == false) return false;
		}
		RPCClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return false;

		// Set up promise and callback

		auto& package = m_Packages[request.indx];
		package.OnResult = [callback](TString&& response) {
			if (callback == nullptr) return;
			callback();
			};

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		// Use '\0' to split the message
		event->Message = input + char();
		m_Reactor->writeAndFlush(event, nullptr);
		return true;
	}

protected:
	friend class RPCClientInboundHandler;
	uint32_t m_PackageID = 0;
	uint32_t m_Buffers = UINT32_MAX;	// bytes from property
	uint32_t m_ReadTimeout = 2000;	// ms from property
	uint32_t m_WriteTimeout = 2000;	// ms from property
	TMutex m_Lock;
	TRef<TCPClientReactor> m_Reactor;
	TMap<uint32_t, invoke_t> m_Packages;
};

class RPCClientInboundHandler : public ChannelInboundHandler
{
public:
	RPCClientInboundHandler(TRaw<RPCClient> client);
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) override;

protected:
	TString m_Buffer;
	TRaw<RPCClient> m_Client;
};