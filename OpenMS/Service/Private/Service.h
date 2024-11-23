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
#include "../IService.h"
#include "../IProperty.h"

class Service :
	public IService,
	public AUTOWIRE(IProperty)
{
public:
	using IService::property;
	TString property(TString const& name) const override;
};