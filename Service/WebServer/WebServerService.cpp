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
#include "WebServerService.h"

MSString WebServerService::identity() const
{
	return "fileserver";
}

void WebServerService::onInit()
{
	ClusterService::onInit();

	bind_get("/static/.*", [=](request_t const& request, response_t& response)
	{
		MS_INFO("%s", request.Url.c_str());
		response.Code = 200;
	});

	HTTPServer::startup();
}

void WebServerService::onExit()
{
	ClusterService::onExit();

	HTTPServer::shutdown();
}

void WebServerService::configureEndpoint(HTTPServer::config_t& config)
{
	config.IP = property(identity() + ".web.ip", MSString("127.0.0.1"));
	config.PortNum  = property(identity() + ".web.port", 80U);
	m_StaticPaths = property(identity() + ".web.static-paths", MSStringList());
}
