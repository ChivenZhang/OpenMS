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
#include "Service.h"
#include "../IEnvironment.h"

int IEnvironment::argc = 0;
char** IEnvironment::argv = nullptr;

TString Service::property(TString const& name) const
{
	auto source = AUTOWIRE_DATA(IProperty);
	if (source == nullptr) return TString();
	return source->property(name);
}