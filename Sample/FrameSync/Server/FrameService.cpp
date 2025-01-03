/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "FrameService.h"

int main(int argc, char* argv[])
{
	return IApplication::Run<FrameService>(argc, argv);
}

void FrameService::onMessage(sync::Message&& msg)
{
	switch (msg.type())
	{
	case sync::MSG_ENTER_BATTLE:
		MS_PRINT("MSG_ENTER_BATTLE");
		break;
	case sync::MSG_MATCH_BATTLE:
		MS_PRINT("MSG_MATCH_BATTLE");
		break;
	case sync::MSG_CANCEL_MATCH:
		MS_PRINT("MSG_CANCEL_MATCH");
		break;
	case sync::MSG_READY_BATTLE:
		MS_PRINT("MSG_READY_BATTLE");
		break;
	case sync::MSG_BEGIN_BATTLE:
		MS_PRINT("MSG_BEGIN_BATTLE");
		break;
	case sync::MSG_END_BATTLE:
		MS_PRINT("MSG_END_BATTLE");
		break;
	case sync::MSG_EXIT_BATTLE:
		MS_PRINT("MSG_EXIT_BATTLE");
		break;
	}
}

void FrameService::onInit()
{
	AUTOWIRE(FrameServer)::bean()->startup();
}

void FrameService::onExit()
{
	AUTOWIRE(FrameServer)::bean()->shutdown();
}