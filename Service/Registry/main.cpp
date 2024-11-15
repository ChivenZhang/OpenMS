/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "RegistryService.h"
#include <csignal>

RegistryService service;

static void on_signal(int signal)
{
	service.shutdown();
}

int main()
{
	::signal(SIGINT, on_signal);
	service.startup();
}