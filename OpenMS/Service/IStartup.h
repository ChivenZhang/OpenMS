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
#include <OpenMS/Service/IEnvironment.h>
#include <csignal>

extern TRef<IService> openms_startup();

int main(int argc, char** argv)
{
	IEnvironment::argc = argc;
	IEnvironment::argv = argv;

	auto service = openms_startup();
	if (service == nullptr) return 1;
	service->startup();

	return 0;
}
#endif