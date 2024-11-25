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

int IService::argc = 0;
char** IService::argv = nullptr;

TString Service::property(TString const& name) const
{
	auto source = AUTOWIRE_DATA(IProperty);
	if (source == nullptr) return TString();
	return source->property(name);
}