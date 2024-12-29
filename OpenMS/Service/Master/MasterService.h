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
	public RESOURCE(MasterConfig)
{
public:
	MSString identity() const override;
	void configureEndpoint(config_t& config) const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	clock_t m_MailUpdateTime = 0;
	MSStringMap<MSSet<MSString>> m_MailRouteMap;
	MSStringMap<MSSet<MSString>> m_MailRouteNewMap;
};
