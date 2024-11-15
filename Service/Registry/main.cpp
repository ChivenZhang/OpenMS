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
#include <nlohmann/json.hpp>

struct User
{
	std::string name;
	std::string password;
};
template<>
bool TTypeC(TString const& s, User& u)
{
	nlohmann::json j = nlohmann::json::parse(s.c_str());
	u.name = j.at("name").get<std::string>();
	u.password = j.at("password").get<std::string>();
	return true;
}
template<>
bool TTypeC(User const& u, TString& s)
{
	nlohmann::json j = { { "name", u.name },{ "password", u.password } };
	s = j.dump();
	return true;
}

void on_signal(int signal);

RegistryService service;

int main(int argc, char* argv[])
{
	signal(SIGINT, on_signal);
	service.startup(argc, argv);

	auto username = service.property<std::string>("registry.user.name");
	auto password = service.property<std::string>("registry.user.password");
	TPrint("1 user: %s pass: %s\n", username.c_str(), password.c_str());
	auto user = service.property<User>("registry.user");
	TPrint("2 user: %s pass: %s\n", user.name.c_str(), user.password.c_str());
}

void on_signal(int signal)
{
	service.shutdown();
}