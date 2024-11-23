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

TRef<IService> openms_startup()
{
	return TNew<RegistryService>();
}

TMutex mutex;
TMutexUnlock unlock;

void RegistryService::startup()
{
	signal(SIGINT, [](int) {
		unlock.notify_all();
		});
	AUTOWIRE(RegistryServer)::bean()->startup();
	TUniqueLock lock(mutex);
	unlock.wait(lock);
	AUTOWIRE(RegistryServer)::bean()->shutdown();
}