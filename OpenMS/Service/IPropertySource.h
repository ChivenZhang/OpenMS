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

/// @brief Interface for classes that provide properties
class OPENMS_API IPropertySource
{
public:
	virtual TString property(TStringView name) const = 0;

	template <class T>
	T property(TStringView name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}
};