#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Service/Master/MasterService.h>
#include <OpenMS/Endpoint/HTTP/HTTPServer.h>
#include <OpenMS/Endpoint/HTTP/HTTPClient.h>

class MasterDemo : public MasterService, public HTTPClient
{
protected:
	void onInit() override;
	void onExit() override;
	void configureEndpoint(HTTPClient::config_t& config) const override;
};