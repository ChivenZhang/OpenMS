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
#include "OpenMS/Service/Private/Service.h"
#include "AuthorityConfig.h"

using RegistryIPTable = MSMap<MSString, MSList<MSString>>;

class AuthorityService :
	public Service,
	public RESOURCE(AuthorityConfig),
	public AUTOWIRE(AuthorityServer),
	public AUTOWIRE(AuthorityClient)
{
public:
	void onInit() override;
	void onExit() override;

protected:
	RegistryIPTable m_IPTables;
};