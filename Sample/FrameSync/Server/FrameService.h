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
#include <OpenMS/Service/Private/Service.h>
#include "../FrameConfig.h"
#include "../Message.pb.h"

class FrameService :
	public Service,
	public RESOURCE(FrameConfig1),
	public AUTOWIRE(FrameServer)
{
public:
	void onMessage(sync::Message && msg);

protected:
	void onInit() override;
	void onExit() override;
};