#include "MasterDemo.h"

void MasterDemo::onInit()
{
	MasterService::onInit();

	HTTPServer::startup();

	HTTPServer::bind(HTTP_GET, "/index", [](HTTPServer::request_t const& request, HTTPServer::response_t& response)
	{
		MS_INFO("handle %s", request.Url.c_str());
		response.Code = HTTP_STATUS_OK;
		response.Body = "Hello,OpenMS!";
	});
}

void MasterDemo::onExit()
{
	MasterService::onExit();

	HTTPServer::shutdown();
}

void MasterDemo::configureEndpoint(HTTPServer::config_t& config) const
{
	config.IP = "0.0.0.0";
	config.PortNum = 9090;
}

OPENMS_RUN(MasterDemo)
