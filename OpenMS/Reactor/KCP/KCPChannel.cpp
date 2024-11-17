/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "KCPChannel.h"

KCPChannel::KCPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uint32_t workID, uv_udp_t* handle, ikcpcb* session)
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

TRaw<ChannelReactor> KCPChannel::getReactor() const
{
	return m_Reactor;
}