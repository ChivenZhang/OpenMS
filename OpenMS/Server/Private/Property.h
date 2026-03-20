#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "OpenMS/Server/IProperty.h"
#define OPENMS_CONFIG_FILE "APPLICATION.json"

class Value : public IValue
{
public:
	using IValue::value;
	MSString value() const override;
	void setValue(MSString const& value);

protected:
	MSString m_Value;
};

class Property : public IProperty
{
public:
	Property();
	MSString property(MSString const& name) const override;

protected:
	MSStringMap<MSString> m_PropertyMap;
};