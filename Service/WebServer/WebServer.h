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
#include <OpenMS/Server/Cluster/ClusterServer.h>
#include <OpenMS/Endpoint/HTTP/HTTPServer.h>
#include "WebConfig.h"

class WebServer : public ClusterServer, public WebConfig
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	void forward(MSString url, HTTPServer::response_t& response);
	void redirect(MSString url, HTTPServer::response_t& response);
	virtual void handle(MSString url, HTTPServer::response_t& response);

protected:
	friend class WebServerErrorHandler;
	uint32_t m_MaxBodySize = 0;
	MSRef<HTTPServer> m_HttpServer;
	MSStringMap<MSString> m_ErrorPages;
	MSStringMap<MSString> m_StaticRoots;
	MSStringMap<MSStringList> m_StaticAlias;
};

OPENMS_RUN(WebServer)