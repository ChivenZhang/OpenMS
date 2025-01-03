#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 19:54:44.
*
* =================================================*/
#include <OpenMS/Service/Cluster/ClusterService.h>

class ClusterDemo1 : public ClusterService
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	MSThread m_Thread;
	MSAtomic<bool> m_Running;
};