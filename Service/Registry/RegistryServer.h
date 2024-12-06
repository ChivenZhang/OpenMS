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
#include "OpenMS/Endpoint/RPC/RPCServer.h"
#include "OpenMS/Endpoint/RPC/RPCClient.h"

using RegistryIPTable = MSMap<MSString, MSVector<MSString>>;

class RegistryServer :
	public RPCServer,
	public AUTOWIRE(IProperty),
	public AUTOWIREN(Value, "iptable")
{
public:
	void configureEndpoint(config_t & config) override;

protected:
	MSMutex m_Lock;
	RegistryIPTable m_IPTables;
};