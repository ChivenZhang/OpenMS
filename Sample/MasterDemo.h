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
#include <OpenMS/Service/Master/MasterService.h>

class MasterDemo : public MasterService
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
};