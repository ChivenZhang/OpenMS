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
	void forward(MSString url, HTTPServer::response_t& response);
	void redirect(MSString url, HTTPServer::response_t& response);

protected:
	friend class WebServerErrorHandler;
	uint32_t m_MaxBodySize = 0;
	MSStringMap<MSString> m_ErrorPages;
	MSStringMap<MSString> m_StaticRoots;
	MSStringMap<MSStringList> m_StaticAlias;
};

OPENMS_RUN(WebServerService)