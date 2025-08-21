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
#include "FileServerService.h"

MSString FileServerService::identity() const
{
	return "fileserver";
}

void FileServerService::onInit()
{
	ClusterService::onInit();

	HTTPServer::startup();
}

void FileServerService::onExit()
{
	ClusterService::onExit();

	HTTPServer::shutdown();
}

void FileServerService::configureEndpoint(HTTPServer::config_t& config)
{
	config.IP = property(identity() + ".static.ip", MSString("127.0.0.1"));
	config.PortNum  = property(identity() + ".static.port", 80U);
}
