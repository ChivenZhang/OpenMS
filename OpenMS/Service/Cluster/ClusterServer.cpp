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
#include "ClusterServer.h"

ClusterServer::ClusterServer(MSString ip, uint16_t port, uint32_t backlog, uint32_t workers)
	:
	m_IP(ip),
	m_PortNum(port),
	m_Backlog(backlog),
	m_Workers(workers)
{
}

void ClusterServer::configureEndpoint(config_t& config)
{
	config.IP = m_IP;
	config.PortNum = m_PortNum;
	config.Backlog = m_Backlog;
	config.Workers = m_Workers;
}