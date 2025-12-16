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
#include "UDPChannel.h"

UDPChannel::UDPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, asio::ip::udp::endpoint handle)
	:
	Channel(reactor, local, remote, workID),
	m_Endpoint(handle)
{
}

asio::ip::udp::endpoint const& UDPChannel::getEndpoint() const
{
	return m_Endpoint;
}
