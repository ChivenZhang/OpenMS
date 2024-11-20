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

struct RemoteClientRequest
{
	uint32_t indx;
	TString name;
	TString args;
	OPENMS_TYPE(RemoteClientRequest, indx, name, args)
};

struct RemoteClientResponse
{
	uint32_t indx;
	TString args;
	OPENMS_TYPE(RemoteClientResponse, indx, args)
};

class RemoteClient :
	public IEndpoint,
	public AUTOWIRE(IProperty),
	public AUTOWIREN(Value, "iptable")
{
public:
	struct config_t
	{
		TString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		TCPClientReactor::callback_tcp_t Callback;
	};

	struct invoke_t
	{
		TRaw<TPromise<bool>> Promise;
		TLambda<void(TString&&)> OnResult;
	};

public:
	void startup() override;
	void shutdown() override;
	virtual void configureEndpoint(config_t & config) = 0;

	template<class T, class... Args, OPENMS_NOT_SAME(T, void)>
	T call(TStringView name, Args... args)
	{
		// Convert arguments to string

		TString input, output;
		if (TTypeC(std::make_tuple(args...), input) == false) return T();
		RemoteClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return T();

		// Set up promise and callback

		TPromise<bool> promise;
		auto& package = m_Packages[request.indx];
		package.Promise = &promise;
		package.OnResult = [&](TString&& response) {
			output = response;
			};
		TFuture<bool> future = promise.get_future();

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		event->Message = input + '\0';
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
		// Convert arguments to string

		TString input;
		if (TTypeC(std::make_tuple(args...), input) == false) return T();
		RemoteClientRequest request;
		request.indx = ++m_PackageID;
		request.name = name;
		request.args = input;
		if (TTypeC(request, input) == false) return T();

		// Set up promise and callback

		TPromise<bool> promise;
		auto& package = m_Packages[request.indx];
		package.Promise = &promise;
		package.OnResult = [](TString&& response) {};
		TFuture<bool> future = promise.get_future();

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		event->Message = input + '\0';
		m_Reactor->writeAndFlush(event, nullptr);

		// Wait for result and return

		future.wait_for(std::chrono::milliseconds(m_ReadTimeout));
	}

protected:
	friend class RemoteClientInboundHandler;
	uint32_t m_PackageID = 0;
	uint32_t m_ReadTimeout = 2000;	// milliseconds
	uint32_t m_WriteTimeout = 2000;	// milliseconds
	TMutex m_Lock;
	TRef<TCPClientReactor> m_Reactor;
	TMap<uint32_t, invoke_t> m_Packages;
};

class RemoteClientInboundHandler : public ChannelInboundHandler
{
public:
	RemoteClientInboundHandler(TRaw<RemoteClient> client);
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) override;

protected:
	TString m_Buffer;
	TRaw<RemoteClient> m_Client;
};