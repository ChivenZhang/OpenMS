#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Endpoint/TCP/TCPServer.h>
#include <OpenMS/Endpoint/TCP/TCPClient.h>
#include <OpenMS/Service/IService.h>

class DemoServer : public TCPServer
{
public:
	void configureEndpoint(config_t& config) override;
};

class DemoClient : public TCPClient, public AUTOWIRE(IService)
{
public:
	void configureEndpoint(config_t & config) override;
};