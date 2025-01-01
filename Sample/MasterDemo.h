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

class MasterDemo : public MasterService, public HTTPServer
{
protected:
	void onInit() override;
	void onExit() override;
	void configureEndpoint(HTTPServer::config_t& config) const override;
};