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
	bool contains(TStringView name) const override;
	using IService::property;
	TString property(TStringView name) const override;

protected:
	TMap<uint32_t, TString> m_PropertyMap;
};