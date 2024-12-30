#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/30 00:48:10.
*
* =================================================*/
#include "Endpoint/RPC/RPCClient.h"

/// @brief 
class ClusterClient : public RPCClient
{
public:
	ClusterClient(MSString ip, uint16_t port, uint32_t workers);

protected:
	void configureEndpoint(config_t& config) const override;

protected:
	const MSString m_IP;
	const uint16_t m_PortNum;
	const uint32_t m_Workers;
};