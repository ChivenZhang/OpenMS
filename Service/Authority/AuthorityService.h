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
	public RESOURCE(AuthorityServer),
	public RESOURCE(AuthorityClient),
	public AUTOWIREN(Value, "iptable")
{
public:
	void startup() override;
	void shutdown() override;

protected:
	TMutex m_Lock;
	TThread m_Thread;
	TMutexUnlock m_Unlock;
	TAtomic<bool> m_Running;
	RegistryIPTable m_IPTables;
};