#pragma once
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
#include <OpenMS/Server/Cluster/ClusterServer.h>
#include <OpenMS/Endpoint/TCP/TCPClient.h>

class DaemonServer : public ClusterServer
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
};

OPENMS_RUN(DaemonServer)