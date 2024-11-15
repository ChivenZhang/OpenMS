#pragma once
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
#include "MS.h"

/// @brief Interface for service
class OPENMS_API IService
{
public:
	virtual ~IService() = default;

	virtual void startup(int argc, char** argv) = 0;

	virtual void shutdown() = 0;

	virtual bool hasProperty(TStringView name) const = 0;

	virtual TString getProperty(TStringView name) const = 0;

	template <class T>
	T getProperty(TStringView name, T const& value = T()) const
	{
		T result;
		if (TText<T>::from_string(getProperty(name), result)) return result;
		return value;
	}
};

/// @brief Interface for provider service
class OPENMS_API IProviderService : public IService
{
public:

};

/// @brief Interface for comsumer service
class OPENMS_API IComsumerService : public IService
{
public:

};