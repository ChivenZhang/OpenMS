#include "DemoService.h"

int main(int argc, char* argv[])
{
	return IApplication::Run<DemoService>(argc, argv);
}

void DemoService::onInit()
{
	auto server = AUTOWIRE(DemoServer)::bean();
	server->startup();

	auto client = AUTOWIRE(DemoClient)::bean();
	client->startup();
}

void DemoService::onExit()
{
	auto server = AUTOWIRE(DemoServer)::bean();
	server->shutdown();

	auto client = AUTOWIRE(DemoClient)::bean();
	client->shutdown();
}