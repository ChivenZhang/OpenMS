#include "MasterDemo.h"

MSString MasterDemo::identity() const
{
	return "master1";
}

void MasterDemo::onInit()
{
	MasterService::onInit();
}

void MasterDemo::onExit()
{
	MasterService::onExit();
}

OPENMS_RUN(MasterDemo)