#include "MasterDemo.h"

void MasterDemo::onInit()
{
	MasterService::onInit();

	HTTPClient::startup();

	// HTTPServer::bind_get(R"(/index.*)", [](auto const& request, auto& response)
	// {
	// 	response.Body = "Hello,OpenMS!";
	// 	response.Code = HTTP_STATUS_OK;
	// });
	//
	// HTTPServer::bind_get(R"(/version)", [](auto const& request, auto& response)
	// {
	// 	response.Body = "1.0.0.0";
	// 	response.Code = HTTP_STATUS_OK;
	// });
	//
	// HTTPServer::bind_get(R"(/api)", [](auto const& request, auto& response)
	// {
	// 	MSStringList api = {"/index", "/version", "/api"};
	// 	response.Body = TTextC::to_string(api);
	// 	response.Code = HTTP_STATUS_OK;
	// });

	HTTPClient::response_t response;
	HTTPClient::call_get({.Url = "/"}, 1000, response);
	MS_INFO("call_get : %d", response.Code);
	MS_INFO("%s", response.Body.c_str());
}

void MasterDemo::onExit()
{
	MasterService::onExit();

	HTTPClient::shutdown();
}

void MasterDemo::configureEndpoint(HTTPClient::config_t& config) const
{
	config.IP = "103.235.47.188";
	config.PortNum = 80;
}

OPENMS_RUN(MasterDemo)
