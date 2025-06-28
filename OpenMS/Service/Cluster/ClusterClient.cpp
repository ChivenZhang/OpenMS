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
#include "ClusterClient.h"

ClusterClient::ClusterClient(MSString ip, uint16_t port, uint32_t workers)
	:
	m_IP(ip),
	m_PortNum(port),
	m_Workers(workers)
{
}

void ClusterClient::configureEndpoint(config_t& config)
{
	config.IP = m_IP;
	config.PortNum = m_PortNum;
	config.Workers = m_Workers;
}
