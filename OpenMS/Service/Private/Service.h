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
#include "OpenMS/Reactor/TCP/TCPServerReactor.h"

class Service : public IService, public AUTOWIRE(IProperty)
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
	using IService::property;
	TString property(TStringView name) const override;

protected:
	TRef<TCPServerReactor> m_Reactor;
};