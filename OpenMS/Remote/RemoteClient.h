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

class RemoteClient :
	public IEndpoint,
	public AUTOWIRE(IProperty),
	public AUTOWIREN(Value, "iptable")
{
public:
	struct config_t
	{
		TString Service;
	};

public:
	RemoteClient();
	~RemoteClient();
	void startup() override;
	void shutdown() override;
	virtual void configureEndpoint(config_t & config) {}

protected:
	TRef<TCPClientReactor> m_RemoteClient;
};