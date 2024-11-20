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
#include "RemoteClient.h"

using IPTable = TMap<TString, TVector<TString>>;

RemoteClient::RemoteClient()
{
	AUTOWIREN(Value, "iptable")::bean()->setValue(R"({"authority":["127.0.0.1:8080"],"gateway":["127.0.0.1:8081"]})");
	startup();
}

RemoteClient::~RemoteClient()
{
	shutdown();
}

void RemoteClient::startup()
{
	auto iptable = AUTOWIREN(Value, "iptable")::bean()->value<IPTable>();
	for (auto& item : iptable)
	{
		std::cout << item.first << std::endl;
		for (auto& ip : item.second)
		{
			std::cout << ip << std::endl;
		}
	}
}

void RemoteClient::shutdown()
{
}
