/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "KCPChannel.h"

KCPChannel::KCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, uv_udp_t* handle, ikcpcb* session)
	:
	Channel(reactor, local, remote, workID),
	m_Handle(handle),
	m_Session(session)
{
}

KCPChannel::~KCPChannel()
{
	ikcp_release(m_Session); m_Session = nullptr;
}

uv_udp_t* KCPChannel::getHandle() const
{
	return m_Handle;
}

ikcpcb* KCPChannel::getSession() const
{
	return m_Session;
}

MSRaw<ChannelReactor> KCPChannel::getReactor() const
{
	return m_Reactor;
}