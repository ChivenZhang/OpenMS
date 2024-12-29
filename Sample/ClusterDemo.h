#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 19:54:44.
*
* =================================================*/
#include <OpenMS/Service/Cluster/ClusterService.h>

class ClusterDemo : public ClusterService
{
protected:
	void onInit() override;
	void onExit() override;
};