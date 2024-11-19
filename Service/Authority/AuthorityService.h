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

#define OPENMS_SERVICE_NAME "authority"

class AuthorityService :
	public Service,
	public RESOURCE(AuthorityConfig),
	public AUTOWIREN(IEndpoint, OPENMS_SERVICE_NAME)
{
public:
	void startup() override;
	void shutdown() override;
};