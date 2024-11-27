#pragma once
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
#include "MS.h"
#include <csignal>

/// @brief Interface for service
class OPENMS_API IService
{
public:
	virtual ~IService() = default;

	virtual int startup() = 0;

	virtual void shutdown() = 0;

	virtual TString property(TString const& name) const = 0;

	template <class T>
	T property(TString const& name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}
};

/// @brief Interface for application
class OPENMS_API IApplication
{
public:
	static int argc;
	static char** argv;

public:
	template <class T, OPENMS_BASE_OF(IService, T)>
	static int Run(int argc, char** argv)
	{
		IApplication::argc = argc;
		IApplication::argv = argv;
		static TRef<IService> service;
		if (service == nullptr) service = TNew<T>();
		signal(SIGINT, [](int) { service->shutdown(); });
		auto result = service->startup();
		signal(SIGINT, nullptr);
		service = nullptr;
		return result;
	}
};