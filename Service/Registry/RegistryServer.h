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
#include "OpenMS/Endpoint/RPC/RPCServer.h"
#include "OpenMS/Endpoint/RPC/RPCClient.h"

using RegistryIPTable = TMap<TString, TVector<TString>>;

class RegistryServer :
	public RPCServer,
	public AUTOWIRE(IProperty),
	public AUTOWIREN(Value, "iptable")
{
public:
	RegistryServer();
	~RegistryServer();
	void configureEndpoint(config_t & config) override;

protected:
	TMutex m_Lock;
	RegistryIPTable m_IPTables;
};