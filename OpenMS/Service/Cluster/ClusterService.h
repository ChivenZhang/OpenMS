#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 15:57:15.
*
* =================================================*/
#include "../Private/Service.h"
#include "Endpoint/RPC/RPCClient.h"
#include "ClusterConfig.h"

/// @brief 
class ClusterService :
	public Service,
	public RPCClient,
	public RESOURCE(ClusterConfig),
	public AUTOWIRE(IMailContext)
{
protected:
	void onInit() override;
	void onExit() override;

public:
	void configureEndpoint(config_t& config) const override;

protected:
	MSMultiMap<MSString, MSString> m_MailRouteMap;
};
