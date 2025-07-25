#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Service/Master/MasterService.h>

class MasterDemo : public MasterService
{
protected:
	void onInit() override;
	void onExit() override;
};

OPENMS_RUN(MasterDemo)