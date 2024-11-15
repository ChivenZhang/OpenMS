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
#include "TCPChannel.h"

TCPChannel::TCPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uv_tcp_t* handle)
	:
	Channel(reactor, local, remote),
	m_Handle(handle)
{
}

uv_tcp_t* TCPChannel::getHandle() const
{
	return m_Handle;
}
