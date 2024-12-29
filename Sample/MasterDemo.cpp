#include "MasterDemo.h"

MSString MasterDemo::identity() const
{
	return "master";
}

void MasterDemo::onInit()
{
	MasterService::onInit();
}

void MasterDemo::onExit()
{
	MasterService::onExit();
}

int main(int argc, char* argv[])
{
	return IApplication::Run<MasterDemo>(argc, argv);
}