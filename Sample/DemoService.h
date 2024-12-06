#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Service/Private/Service.h>
#include "DemoServer.h"

class DemoConfig
	:
	public RESOURCE(DemoServer),
	public RESOURCE(DemoClient)
{
};

class DemoService :
	public Service,
	public RESOURCE(DemoConfig),
	public AUTOWIRE(DemoServer),
	public AUTOWIRE(DemoClient)
{
public:
	void onInit() override;
	void onExit() override;
};