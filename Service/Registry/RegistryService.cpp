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
#include <OpenMS/Service/IStartup.h>

int openms_main(int argc, char** argv)
{
	RegistryService service;
	service.startup();
	return 0;
}

TMutex mutex;
TMutexUnlock unlock;

int RegistryService::startup()
{
	signal(SIGINT, [](int) { unlock.notify_all(); });

	AUTOWIRE(RegistryServer)::bean()->startup();

	TUniqueLock lock(mutex); unlock.wait(lock);

	AUTOWIRE(RegistryServer)::bean()->shutdown();

	return 0;
}