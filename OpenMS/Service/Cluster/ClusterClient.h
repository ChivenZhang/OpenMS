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
#include "Endpoint/RPC/RPCClient.h"

/// @brief 
class ClusterClient : public RPCClient
{
public:
	ClusterClient(MSString ip, uint16_t port, uint32_t workers);

protected:
	void configureEndpoint(config_t& config) override;

protected:
	const MSString m_IP;
	const uint16_t m_PortNum;
	const uint32_t m_Workers;
};