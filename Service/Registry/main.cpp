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

struct User
{
	std::string name;
	std::string password;
	OPENMS_TYPE(User, name, password)
};

void on_signal(int signal);

RegistryService service;

int main(int argc, char* argv[])
{
	signal(SIGINT, on_signal);
	service.startup(argc, argv);

#if 1 // test code

	TPrint("%d", service.property<uint16_t>("registry.server.port"));

	auto user = service.property<User>("registry.user");
	TPrint("1 user: %s pass: %s\n", user.name.c_str(), user.password.c_str());

	auto users = service.property<TVector<User>>("registry.users");
	for (auto& user : users)
	{
		TPrint("2 user: %s pass: %s\n", user.name.c_str(), user.password.c_str());
	}

#endif
}

void on_signal(int signal)
{
	service.shutdown();
}