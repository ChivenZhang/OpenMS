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

int main(int argc, char* argv[])
{
	return IApplication::Run<FrameService>(argc, argv);
}

void FrameService::onMessage(sync::Message&& msg)
{
	switch (msg.type())
	{
	case sync::MSG_ENTER_BATTLE:
		TPrint("MSG_ENTER_BATTLE");
		break;
	case sync::MSG_MATCH_BATTLE:
		TPrint("MSG_MATCH_BATTLE");
		break;
	case sync::MSG_READY_BATTLE:
		TPrint("MSG_READY_BATTLE");
		break;
	case sync::MSG_BEGIN_BATTLE:
		TPrint("MSG_BEGIN_BATTLE");
		break;
	case sync::MSG_END_BATTLE:
		TPrint("MSG_END_BATTLE");
		break;
	case sync::MSG_EXIT_BATTLE:
		TPrint("MSG_EXIT_BATTLE");
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