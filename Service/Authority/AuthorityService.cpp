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
#include "AuthorityService.h"

int main(int argc, char* argv[])
{
	return IApplication::Run<AuthorityService>(argc, argv);
}

void AuthorityService::onInit()
{
	auto serviceName = property("authority.name");
	auto server = AUTOWIRE(AuthorityServer)::bean();
	auto client = AUTOWIRE(AuthorityClient)::bean();

	server->startup();
	client->startup();

	auto address = server->address().lock();
	if (address) client->call<TString>("registry/register", 1000, serviceName, address->getString());

	auto update_func = [=](uint32_t handle) {
		auto address = server->address().lock();
		if (address)  client->call<TString>("registry/renew", 100, serviceName, address->getString());
		client->async<TString>("registry/query", 100, {}, [](TString&& result) {
			TPrint("query result: %s", result.c_str());
			});
		};
	startTimer(0, 1000, update_func);
}

void AuthorityService::onExit()
{
	auto server = AUTOWIRE(AuthorityServer)::bean();
	auto client = AUTOWIRE(AuthorityClient)::bean();

	server->shutdown();
	client->shutdown();
}