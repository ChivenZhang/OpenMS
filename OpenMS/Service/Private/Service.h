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
#include "OpenMS/Channel/TCP/TCPServerReactor.h"

class Service : public IService
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
	using IService::property;
	TString property(TStringView name) const override;

	bool execute(TStringView service, TStringView method)
	{
		return false;
	}

protected:
	TRef<TCPServerReactor> m_Reactor;
	TMap<uint32_t, TString> m_PropertyMap;
};