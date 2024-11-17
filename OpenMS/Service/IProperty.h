#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "MS.h"

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

/// @brief Interface for property
class OPENMS_API IProperty
{
public:
	virtual TString property(TStringView name) const = 0;

	template <class T>
	T property(TStringView name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}
};