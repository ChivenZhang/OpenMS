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
#include "TCPChannel.h"
#include "../Private/ChannelReactor.h"

TCPChannel::TCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, uv_tcp_t* handle)
	:
	Channel(reactor, local, remote, workID),
	m_Handle(handle)
{
}

uv_tcp_t* TCPChannel::getHandle() const
{
	return m_Handle;
}
