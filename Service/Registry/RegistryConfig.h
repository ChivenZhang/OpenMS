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
#include "OpenMS/Service/Private/Property.h"
#include "OpenMS/Endpoint/RPC/RPCServer.h"
#include "RegistryServer.h"

class RegistryConfig :
	public RESOURCE2(Property, IProperty),
	public RESOURCE2N(Value, IValue, "iptable"),
	public RESOURCE2N(RegistryServer, IEndpoint, "registry-server"),
	public RESOURCE2N(RegistryClient, RPCClient, "registry-client")
{
};