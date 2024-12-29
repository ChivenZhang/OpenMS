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

/// @brief Interface for properties
class OPENMS_API IProperty
{
public:
	virtual ~IProperty() = default;
	virtual MSString property(MSString const& name) const = 0;

	template <class T>
	T property(MSString const& name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}
};

/// @brief Interface for value
class OPENMS_API IValue
{
public:
	virtual ~IValue() = default;
	virtual MSString value() const = 0;

	template <class T>
	T value(T const& value = T()) const
	{
		return TTextC<T>::from_string(this->value(), T());
	}
};