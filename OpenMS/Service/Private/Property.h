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
#include "../IProperty.h"
#include "../Autowired.h"

class Value : public IValue
{
public:
	TString value() const override;
	void setValue(TString const& value);

protected:
	TString m_Value;
};

class Property : public IProperty
{
public:
	Property();
	TString property(TStringView name) const override;

protected:
	TMap<TString, TString> m_PropertyMap;
};