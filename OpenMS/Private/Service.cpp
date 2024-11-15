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

#include "Service.h"
#include <filesystem>
#include <nlohmann/json.hpp>

void Service::startup(int argc, char** argv)
{
	auto directory = std::filesystem::path(argv[0]).parent_path().generic_string();
	auto configFile = directory + "/" + "openms.json";
}

void Service::shutdown()
{
}

bool Service::hasProperty(TStringView name) const
{
	return false;
}

TString Service::getProperty(TStringView name) const
{
	return TString();
}
