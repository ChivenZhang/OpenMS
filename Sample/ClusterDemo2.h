#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 21:47:04.
*
* =================================================*/
#include <OpenMS/Service/Cluster/ClusterService.h>

class ClusterDemo2 : public ClusterService
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
};