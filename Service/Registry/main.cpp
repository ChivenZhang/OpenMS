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

int main(int argc, char* argv[])
{
	signal(SIGINT, on_signal);
	service.startup(argc, argv);

	auto port = service.property<uint16_t>("registry.server.port");
	TPrint("%d", port);
	auto username = service.property<std::string>("registry.user.name");
	auto password = service.property<std::string>("registry.user.password");
	TPrint("%s %s", username.c_str(), password.c_str());
}

void on_signal(int signal)
{
	service.shutdown();
}