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
#include "GatewayService.h"
#include <OpenMS/Service/IBootstrap.h>

TRef<IService> openms_bootstrap()
{
	return TNew<GatewayService>();
}

void GatewayService::startup()
{
	Service::startup();
}

void GatewayService::shutdown()
{
	Service::shutdown();
}