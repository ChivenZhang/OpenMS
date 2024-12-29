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
#include "ClusterClient.h"

ClusterClient::ClusterClient(MSString ip, uint16_t port)
	:
	m_IP(ip),
	m_PortNum(port)
{
}

void ClusterClient::configureEndpoint(config_t& config) const
{
	config.IP = m_IP;
	config.PortNum = m_PortNum;
	config.Workers = 1;
}
