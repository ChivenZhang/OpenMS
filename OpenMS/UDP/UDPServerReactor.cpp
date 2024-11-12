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
#include "UDPServerReactor.h"

UDPServerReactor::UDPServerReactor(TRef<ISocketAddress> address, bool broadcast, bool multicast, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback)
{
}

void UDPServerReactor::startup()
{
}

void UDPServerReactor::shutdown()
{
}

void UDPServerReactor::onConnect(TRef<Channel> channel)
{
}

void UDPServerReactor::onDisconnect(TRef<Channel> channel)
{
}

void UDPServerReactor::on_connect(uv_connect_t* req, int status)
{
}

void UDPServerReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
}

void UDPServerReactor::on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
}

void UDPServerReactor::on_stop(uv_async_t* handle)
{
}
