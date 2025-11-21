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
class OPENMS_API IStartup
{
public:
	template <class T, OPENMS_BASE_OF(IServer, T)>
	static int Run(int argc, char* argv[])
	{
		printf(OPENMS_LOGO);
		IStartup::Argc = argc;
		IStartup::Argv = argv;
		RESOURCE2_DATA(T, IServer);
		auto service = AUTOWIRE_DATA(IServer);
		signal(SIGINT, [](int) { AUTOWIRE_DATA(IServer)->shutdown(); });
		signal(SIGTERM, [](int) { AUTOWIRE_DATA(IServer)->shutdown(); });
		auto result = service->startup();
		signal(SIGINT, nullptr);
		signal(SIGTERM, nullptr);
		IStartup::Argc = 0;
		IStartup::Argv = nullptr;
		return result;
	}

public:
	static int Argc;
	static char** Argv;
};

#define OPENMS_RUN(T) int main(int argc, char* argv[]) { return IStartup::Run<T>(argc, argv); }
