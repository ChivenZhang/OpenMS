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
#include <MS.h>
#include <csignal>

/// @brief Interface for bootstrap
class OPENMS_API IStartup
{
public:
	template <class T, OPENMS_BASE_OF(IServer, T)>
	static int Run(int argc, char* argv[])
	{
		std::printf(OPENMS_LOGO);
		spdlog::set_pattern("%@:%!()\n%Y-%m-%d %H:%M:%S.%e %L [%t] --- %^%v%$");
		for (auto i = 0; i < argc; i++) MS_INFO("argv[%d]: %s", i, argv[i]);
		IStartup::Argc = argc;
		IStartup::Argv = argv;
		RESOURCE2_DATA(T, IServer);
		auto service = AUTOWIRE_DATA(IServer);
		signal(SIGINT, [](int) { AUTOWIRE_DATA(IServer)->shutdown(); });
		signal(SIGTERM, [](int) { AUTOWIRE_DATA(IServer)->shutdown(); });
		signal(SIGABRT, [](int) { AUTOWIRE_DATA(IServer)->shutdown(); });
		auto result = service->startup();
		signal(SIGINT, nullptr);
		signal(SIGTERM, nullptr);
		signal(SIGABRT, nullptr);
		IStartup::Argc = 0;
		IStartup::Argv = nullptr;
		return result;
	}

public:
	static int Argc;
	static char** Argv;
};

#define OPENMS_RUN(T) int main(int argc, char* argv[]) { return IStartup::Run<T>(argc, argv); }
