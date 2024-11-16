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
#define OPENMS_CONFIG_FILE "application.json"

/// @brief Interface for service
class OPENMS_API IService
{
public:
	static TRaw<IService> Instance();

public:
	virtual ~IService() = default;

	virtual void startup(int argc, char** argv) = 0;

	virtual void shutdown() = 0;

	virtual TString property(TStringView name) const = 0;

	template <class T>
	T property(TStringView name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}

protected:
	static TRaw<IService> m_Instance;
};

/// @brief Interface for provider service
class OPENMS_API IProviderService : public virtual IService
{
public:

};

/// @brief Interface for comsumer service
class OPENMS_API IComsumerService : public virtual IService
{
public:

};