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

void on_signal(int signal);

RegistryService service;

int main()
{
	::signal(SIGINT, on_signal);
	service.startup();
}

void on_signal(int signal)
{
	service.shutdown();
}