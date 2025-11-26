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

#include "Server/Private/Service.h"

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
	MSAtomic<uint32_t> m_ClientCount;
};

class GatewayClient : public Service
{
public:
	explicit GatewayClient(MSRef<IChannel> channel);

protected:
	MSString onRequest(MSStringView request) override;
	void onResponse(MSStringView response) override;

protected:
	MSRef<IChannel> m_Channel;
};

OPENMS_RUN(GatewayServer)