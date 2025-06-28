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

TCPChannel::TCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workid, uv_tcp_t* handle)
	:
	Channel(reactor, local, remote, workid),
	m_Handle(handle)
{
}

uv_tcp_t* TCPChannel::getHandle() const
{
	return m_Handle;
}
