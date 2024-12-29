#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "MS.h"
#include <iocpp.h>

#define OPENMS_LOGO \
(R"(
  ___                   __  __ ____  
 / _ \ _ __   ___ _ __ |  \/  / ___| 
| | | | '_ \ / _ \ '_ \| |\/| \___ \ 
| |_| | |_) |  __/ | | | |  | |___) |
 \___/| .__/ \___|_| |_|_|  |_|____/ 
======|_|============================

:: OpenMS ::                (v1.0.0)

)")

/// @brief Interface for service
class OPENMS_API IService
{
public:
	virtual ~IService() = default;

	virtual int startup() = 0;

	virtual void shutdown() = 0;

	virtual MSString identity() const = 0;

	virtual void sendEvent(MSLambda<void()>&& event) = 0;

	virtual uint32_t startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void(uint32_t handle)>&& task) = 0;

	virtual bool stopTimer(uint32_t handle) = 0;

	virtual MSString property(MSString const& name) const = 0;

	template <class T>
	T property(MSString const& name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}
};

#include <csignal>

/// @brief Interface for application
class OPENMS_API OpenMS
{
public:
	template <class T, OPENMS_BASE_OF(IService, T)>
	static int Run(int argc, char* argv[])
	{
		printf(OPENMS_LOGO);
		OpenMS::Argc = argc;
		OpenMS::Argv = argv;
		RESOURCE2_DATA(T, IService);
		auto service = AUTOWIRE_DATA(IService);
		signal(SIGINT, [](int) { AUTOWIRE_DATA(IService)->shutdown(); });
		signal(SIGTERM, [](int) { AUTOWIRE_DATA(IService)->shutdown(); });
		auto result = service->startup();
		signal(SIGINT, nullptr);
		signal(SIGTERM, nullptr);
		OpenMS::Argc = 0;
		OpenMS::Argv = nullptr;
		return result;
	}

public:
	static int Argc;
	static char** Argv;
};
