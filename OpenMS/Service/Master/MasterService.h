#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/29 15:56:12.
*
* =================================================*/
#include "../Private/Service.h"
#include "MasterConfig.h"
#include "Endpoint/RPC/RPCServer.h"

/// @brief 
class MasterService :
	public Service,
	public RPCServer,
	public RESOURCE(MasterConfig),
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
