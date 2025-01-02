#include "MasterDemo.h"

void MasterDemo::onInit()
{
	MasterService::onInit();

	HTTPServer::startup();

	HTTPServer::bind_get(R"(/index.*)", [](auto const& request, auto& response)
	{
		response.Body = "Hello,OpenMS!";
		response.Code = HTTP_STATUS_OK;
	});

	HTTPServer::bind_get(R"(/version)", [](auto const& request, auto& response)
	{
		response.Body = "1.0.0.0";
		response.Code = HTTP_STATUS_OK;
	});

	HTTPServer::bind_get(R"(/api)", [](auto const& request, auto& response)
	{
		MSStringList api = {"/index", "/version", "/api"};
		response.Body = TTextC::to_string(api);
		response.Code = HTTP_STATUS_OK;
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
