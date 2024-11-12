/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "UDPChannel.h"

UDPChannel::UDPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uv_udp_t* handle)
	:
	Channel(reactor, local, remote),
	m_Handle(handle)
{
}

uv_udp_t* UDPChannel::getHandle() const
{
	return m_Handle;
}
