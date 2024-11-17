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
#include "../Autowired.h"
#include "../IPropertySource.h"
#include "OpenMS/Reactor/TCP/TCPServerReactor.h"

class Service : public IService, public AUTOWIRE(IPropertySource, "application")
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
	using IService::property;
	TString property(TStringView name) const override;

protected:
	TRef<TCPServerReactor> m_Reactor;
};