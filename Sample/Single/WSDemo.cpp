/*=================================================
* Copyright © 2020-2026 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Endpoint/WS/WSServer.h>

int main()
{
	WSServer server({
		.IP = "127.0.0.1",
		.PortNum = 9090,
	});
	server.bind_connect([](MSStringView session)
	{
		MS_INFO("connect: %.*s", (int)session.size(), session.data());
	});
	server.bind_disconnect([](MSStringView session)
	{
		MS_INFO("disconnect: %.*s", (int)session.size(), session.data());
	});
	server.bind_message([](MSStringView msg, bool end)
	{
		MS_INFO("message: %.*s %u", (int)msg.size(), msg.data(), end);
	});
	server.bind_binary([](MSStringView msg, bool end)
	{
		MS_INFO("binary: %.*s %u", (int)msg.size(), msg.data(), end);
	});

	server.startup();
	system("pause");
	server.shutdown();
	return 0;
}