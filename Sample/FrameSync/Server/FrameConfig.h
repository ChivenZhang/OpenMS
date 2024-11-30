#pragma once
#include "../FrameConfig.h"
#include "FrameServer.h"
#include "UserManager.h"
#include "PlayerManager.h"
#include "BattleManager.h"

class FrameConfig
	:
	public RESOURCE(FrameServer),
	public RESOURCE(UserManager),
	public RESOURCE(PlayerManager),
	public RESOURCE(BattleManager)
{
};