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

#include "Endpoint/TCP/TCPClient.h"

class BusinessServer : public ClusterServer
{
public:
	MSString identity() const override;

protected:
	void onInit() override;
	void onExit() override;

protected:

};

OPENMS_RUN(BusinessServer)