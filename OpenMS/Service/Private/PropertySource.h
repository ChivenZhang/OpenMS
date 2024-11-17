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
#include "../IPropertySource.h"
#include "../Autowired.h"

class PropertySource : public IPropertySource
{
public:
	PropertySource();
	TString property(TStringView name) const override;

protected:
	TMap<uint32_t, TString> m_PropertyMap;
};