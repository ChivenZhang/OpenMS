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
#include "StatusClient.h"
#include <OpenMS/Service/IStartup.h>

int openms_main(int argc, char** argv)
{
	StatusClient server;
	return server.startup();
}

int StatusClient::startup()
{
	return 0;
}