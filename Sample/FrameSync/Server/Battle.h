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
#include <OpenMS/MS.h>
#include "Player.h"

class Battle
{
public:
	uint32_t ID;
	MSList<MSHnd<Player>> Players;
};