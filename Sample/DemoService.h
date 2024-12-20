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

#include <OpenMS/Mailbox/Private/MailContext.h>

class DemoConfig
	:
	public RESOURCE(DemoServer),
	public RESOURCE(DemoClient),
	public RESOURCE(MailContext)
{
};

class DemoService :
	public Service,
	public RESOURCE(DemoConfig),
	public AUTOWIRE(DemoServer),
	public AUTOWIRE(DemoClient),
	public AUTOWIRE(MailContext)
{
public:
	void onInit() override;
	void onExit() override;
};