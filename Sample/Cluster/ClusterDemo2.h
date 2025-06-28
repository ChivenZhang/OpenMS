#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
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

OPENMS_RUN(ClusterDemo2)