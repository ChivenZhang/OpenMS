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

/// @brief Interface for service
class OPENMS_API IService
{
public:
	virtual ~IService() = default;

	virtual void startup() = 0;

	virtual TString property(TString const& name) const = 0;

	template <class T>
	T property(TString const& name, T const& value = T()) const
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