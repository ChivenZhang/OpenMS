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
#include "KCPClientReactor.h"

KCPClientReactor::KCPClientReactor(TRef<ISocketAddress> address, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback)
{
}

void KCPClientReactor::startup()
{
}

void KCPClientReactor::shutdown()
{
}

void KCPClientReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

void KCPClientReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

void KCPClientReactor::onConnect(TRef<Channel> channel)
{
}

void KCPClientReactor::onDisconnect(TRef<Channel> channel)
{
}

void KCPClientReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
}

void KCPClientReactor::on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const sockaddr* addr, unsigned flags)
{
}

void KCPClientReactor::on_send(uv_udp_t* handle)
{
}

void KCPClientReactor::on_stop(uv_async_t* handle)
{
}
