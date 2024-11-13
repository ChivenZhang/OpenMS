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
#include "KCPServerReactor.h"

KCPServerReactor::KCPServerReactor(TRef<ISocketAddress> address, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback)
{
}

void KCPServerReactor::startup()
{
}

void KCPServerReactor::shutdown()
{
}

void KCPServerReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

void KCPServerReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

void KCPServerReactor::onConnect(TRef<Channel> channel)
{
}

void KCPServerReactor::onDisconnect(TRef<Channel> channel)
{
}

void KCPServerReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
}

void KCPServerReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* addr, unsigned flags)
{
}

void KCPServerReactor::on_send(uv_udp_t* handle)
{
}

void KCPServerReactor::on_stop(uv_async_t* handle)
{
}
