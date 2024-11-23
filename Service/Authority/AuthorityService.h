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
#include "OpenMS/Service/Private/Service.h"
#include "AuthorityConfig.h"

using RegistryIPTable = TMap<TString, TVector<TString>>;

class AuthorityService :
	public Service,
	public RESOURCE(AuthorityConfig),
	public AUTOWIREN(Value, "iptable"),
	public AUTOWIREN(Value, "service"),
	public AUTOWIRE(AuthorityServer),
	public AUTOWIRE(AuthorityClient),
	public AUTOWIRE(Timer)
{
public:
	void startup() override;

protected:
	RegistryIPTable m_IPTables;
};