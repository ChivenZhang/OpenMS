/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Endpoint/RPC/RPCServer.h>
#include <OpenMS/Endpoint/RPC/RPCClient.h>

int main()
{
	auto server = MSNew<RPCServer>(RPCServer::config_t{
		.IP = "127.0.0.1",
		.PortNum = 8080,
	});
	auto client = MSNew<RPCClient>(RPCClient::config_t{
		.IP = "127.0.0.1",
		.PortNum = 8080,
	});
	server->bind("echo", [](MSString const& text) { MS_INFO("echo: %s", text.c_str()); });
	server->startup();
	client->startup();
	client->call<void>("echo", 1000, MSString("Hello,OpenMS!"));
	client->async<void>("echo", 1000, MSTuple{ MSString("Hello,OpenMS!") }, []()
	{
		MS_INFO("echo: return");
	});

	system("pause");

	server->shutdown();
	client->shutdown();
	return 0;
}