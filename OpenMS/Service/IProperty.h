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
#include <iocpp.h>

/// @brief Interface for properties
class OPENMS_API IProperty
{
public:
	virtual TString property(TString const& name) const = 0;

	template <class T>
	T property(TString const& name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}
};

/// @brief Interface for value
class OPENMS_API IValue
{
public:
	virtual TString value() const = 0;

	template <class T>
	T value(T const& value = T()) const
	{
		return TTextC<T>::from_string(this->value(), T());
	}
};