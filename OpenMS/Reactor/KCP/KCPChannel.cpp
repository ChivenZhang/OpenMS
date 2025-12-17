/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "KCPChannel.h"

KCPChannel::KCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, ikcpcb* session, asio::ip::udp::socket* socket, asio::ip::udp::endpoint handle)
	:
	Channel(reactor, local, remote, workID),
	m_Socket(socket),
	m_Session(session),
	m_Endpoint(handle)
{
}

KCPChannel::~KCPChannel()
{
	::ikcp_release(m_Session); m_Session = nullptr;
}

MSRaw<ChannelReactor> KCPChannel::getReactor() const
{
	return this->m_Reactor;
}

asio::ip::udp::socket* KCPChannel::getSocket() const
{
	return m_Socket;
}

ikcpcb* KCPChannel::getSession() const
{
	return m_Session;
}

asio::ip::udp::endpoint const& KCPChannel::getEndpoint() const
{
	return m_Endpoint;
}
