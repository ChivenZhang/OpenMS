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
#include "../IService.h"

class Service : public IService
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
	bool hasProperty(TStringView name) const override;
	TString getProperty(TStringView name) const override;
};