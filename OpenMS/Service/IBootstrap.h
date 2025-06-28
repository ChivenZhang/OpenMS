#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include <csignal>

/// @brief Interface for bootstrap
class OPENMS_API IBootstrap
{
public:
	template <class T, OPENMS_BASE_OF(IService, T)>
	static int Run(int argc, char* argv[])
	{
		printf(OPENMS_LOGO);
		IBootstrap::Argc = argc;
		IBootstrap::Argv = argv;
		RESOURCE2_DATA(T, IService);
		auto service = AUTOWIRE_DATA(IService);
		signal(SIGINT, [](int) { AUTOWIRE_DATA(IService)->shutdown(); });
		signal(SIGTERM, [](int) { AUTOWIRE_DATA(IService)->shutdown(); });
		auto result = service->startup();
		signal(SIGINT, nullptr);
		signal(SIGTERM, nullptr);
		IBootstrap::Argc = 0;
		IBootstrap::Argv = nullptr;
		return result;
	}

public:
	static int Argc;
	static char** Argv;
};

#define OPENMS_RUN(T) int main(int argc, char* argv[]) { return IBootstrap::Run<T>(argc, argv); }