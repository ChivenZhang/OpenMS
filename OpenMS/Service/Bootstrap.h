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
#ifndef OPENMS_SHARED_LIBRARY
#include <OpenMS/Service/IService.h>
#include <OpenMS/Service/Autowired.h>
#include <OpenMS/Service/IEnvironment.h>
#include <csignal>

TMutex mutex;
TMutexUnlock unlock;
TRef<IService> service;
extern void openms_signal(int signal) { unlock.notify_all(); }
extern TRef<IService> openms_bootstrap();

int main(int argc, char** argv)
{
	IEnvironment::argc = argc;
	IEnvironment::argv = argv;
	signal(SIGINT, openms_signal);

	service = openms_bootstrap();
	if (service == nullptr) return 1;
	service->startup(argc, argv);
	TUniqueLock lock(mutex);
	unlock.wait(lock);
	service->shutdown();
	return 0;
}
#endif