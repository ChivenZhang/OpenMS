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
class Service;

class BackendServer : public ClusterServer
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;
	virtual MSRef<Service> onCreatingSpace(uint32_t spaceID, uint32_t gameID);

protected:
	MSHnd<Service> m_SpaceService;
};