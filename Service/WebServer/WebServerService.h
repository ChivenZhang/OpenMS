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
#include <OpenMS/Endpoint/HTTP/HTTPServer.h>
#include "WebServerConfig.h"

class WebServerService
	:
	public ClusterService,
	public HTTPServer,
	public WebServerConfig
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	void configureEndpoint(HTTPServer::config_t& config) override;

protected:
	uint32_t m_MaxBodySize = 0;
	uint32_t m_MaxBufferSize = 0;
	MSStringMap<MSStringList> m_StaticRoots;
	MSStringMap<MSString> m_StaticAlias;
};

OPENMS_RUN(WebServerService)