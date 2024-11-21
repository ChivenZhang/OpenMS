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
#include "OpenMS/Remote/RemoteServer.h"
#include "OpenMS/Remote/RemoteClient.h"

using RegistryIPTable = TMap<TString, TVector<TString>>;

class RegistryServer :
	public RemoteServer,
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

class RegistryClient :
	public RemoteClient,
	public AUTOWIRE(IProperty)
{
public:
	RegistryClient();
	~RegistryClient();
	void configureEndpoint(config_t & config) override;
};