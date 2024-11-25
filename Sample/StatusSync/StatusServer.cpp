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
#include "StatusServer.h"
#include <OpenMS/Service/IStartup.h>

int openms_main(int argc, char** argv)
{
	StatusServer server;
	return server.startup();
}

int StatusServer::startup()
{
	return 0;
}
