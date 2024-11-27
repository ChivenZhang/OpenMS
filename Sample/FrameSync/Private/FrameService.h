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
#include <OpenMS/Reactor/TCP/TCPServerReactor.h>
#include <OpenMS/Reactor/UDP/UDPServerReactor.h>
#include "FrameConfig.h"

class FrameService :
	public Service,
	public RESOURCE(FrameConfig)
{
protected:
	void onStartup() override;
	void onShutdown() override;
};