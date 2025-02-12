#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/30 01:04:22.
*
* =================================================*/
#include "Endpoint/RPC/RPCServer.h"

/// @brief
class ClusterServer : public RPCServer
{
public:
	ClusterServer(MSString ip, uint16_t port, uint32_t backlog, uint32_t workers);

protected:
	void configureEndpoint(config_t& config) override;

protected:
	const MSString m_IP;
	const uint16_t m_PortNum;
	const uint32_t m_Backlog, m_Workers;
};