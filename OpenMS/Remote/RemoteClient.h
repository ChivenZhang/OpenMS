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

struct RemoteClientRequest
{
	TString name;
	TString args;
	OPENMS_TYPE(RemoteClientRequest, name, args)
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

public:
	void startup() override;
	void shutdown() override;
	virtual void configureEndpoint(config_t & config) = 0;

	template<class T, class... Args, OPENMS_NOT_SAME(T, void)>
	T invoke(TStringView name, Args... args)
	{
		// Convert input arguments to string

		TString input, output;
		if (TTypeC(std::make_tuple(args...), input) == false) return T();
		RemoteClientRequest package;
		package.name = name;
		package.args = input;
		TTypeC(package, input);

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		event->Message = input + '\0';
		m_Reactor->writeAndFlush(event, nullptr);

		// TODO: Wait for result (Aysnc)

		T result;
		if (TTypeC(output, result) == false) return T();
		return result;
	}

	template<class T, class... Args, OPENMS_IS_SAME(T, void)>
	T invoke(TStringView name, Args... args)
	{
		// Convert input arguments to string

		TString input, output;
		if (TTypeC(std::make_tuple(args...), input) == false) return;
		RemoteClientRequest package;
		package.name = name;
		package.args = input;
		TTypeC(package, input);

		// Send input to remote server

		auto event = TNew<IChannelEvent>();
		event->Message = input + '\0';
		m_Reactor->writeAndFlush(event, nullptr);

		// TODO: Wait for result (Aysnc)
	}

protected:
	TRef<TCPClientReactor> m_Reactor;
};