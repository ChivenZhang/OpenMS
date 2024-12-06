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
#include "UDPChannel.h"

UDPChannel::UDPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, uv_udp_t* handle)
	:
	Channel(reactor, local, remote, workID),
	m_Handle(handle)
{
}

uv_udp_t* UDPChannel::getHandle() const
{
	return m_Handle;
}
