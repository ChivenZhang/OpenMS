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
#include "FrameService.h"

int main(int argc, char** argv)
{
	return IApplication::Run<FrameService>(argc, argv);
}

void FrameService::onStartup()
{
	startTimer(1000, 1000 / 15, [frame = 0U](uint32_t handle) mutable {
		TPrint("fixed update %d", ++frame);
		});
}

void FrameService::onShutdown()
{
}