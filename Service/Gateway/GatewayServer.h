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

class GatewayServer : public ClusterServer
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:
	uint32_t m_KeepAlive = 0;	// For reconnect
	MSRef<IEndpoint> m_TCPServer;
};

OPENMS_RUN(GatewayServer)