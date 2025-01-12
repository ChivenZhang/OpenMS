#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
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
class MasterService
	:
	public Service,
	public RPCServer,
	public RESOURCE(MasterConfig)
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	void configureEndpoint(config_t& config) final;

protected:
	MSStringMap<MSSet<MSString>> m_MailRouteMap;
	MSStringMap<MSSet<MSString>> m_MailRouteNewMap;
	std::chrono::time_point<std::chrono::system_clock> m_MailUpdateTime;
};
