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

class AuthorityServer :
	public RPCServer,
	public AUTOWIRE(IProperty)
{
public:
	AuthorityServer();
	~AuthorityServer();
	void configureEndpoint(config_t & config) override;
};

class AuthorityClient :
	public RPCClient,
	public AUTOWIRE(IProperty)
{
public:
	AuthorityClient();
	~AuthorityClient();
	void configureEndpoint(config_t & config) override;

protected:

};