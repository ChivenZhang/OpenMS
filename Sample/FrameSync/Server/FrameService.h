#pragma once
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
#include <OpenMS/Service/Private/Service.h>
#include "FrameConfig.h"

class FrameService :
	public Service,
	public RESOURCE(FrameConfig),
	public AUTOWIRE(FrameServer),
	public AUTOWIRE(UserManager),
	public AUTOWIRE(PlayerManager),
	public AUTOWIRE(BattleManager)
{
public:
	void onMessage(sync::Message && msg);

protected:
	void onInit() override;
	void onExit() override;
};