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
#include "RegistryService.h"
#include <OpenMS/Service/IBootstrap.h>

TRef<IService> openms_bootstrap()
{
	return TNew<RegistryService>();
}

void RegistryService::startup()
{
	Service::startup();

#ifdef OPENMS_TEST

	auto server = RESOURCE(RegistryServer)::bean();
	server->bind("echo", [=](TString text) {
		TPrint("%s", text.c_str());
		});

#endif
}

void RegistryService::shutdown()
{
	Service::shutdown();
}