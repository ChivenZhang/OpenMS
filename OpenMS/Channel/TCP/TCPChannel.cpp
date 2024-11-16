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
#include "../Private/ChannelReactor.h"

TCPChannel::TCPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uint32_t workid, uv_tcp_t* handle)
	:
	Channel(reactor, local, remote, workid),
	m_Handle(handle)
{
}

uv_tcp_t* TCPChannel::getHandle() const
{
	return m_Handle;
}

void TCPChannel::close()
{
	if (m_Reactor->running() == false) return;
	uv_close((uv_handle_t*)m_Handle, nullptr);
	Channel::close();
}
