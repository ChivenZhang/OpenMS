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
#include "UDPClientReactor.h"

UDPClientReactor::UDPClientReactor(TRef<ISocketAddress> address, bool broadcast, bool multicast, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback)
{
}

void UDPClientReactor::startup()
{
}

void UDPClientReactor::shutdown()
{
}

void UDPClientReactor::onConnect(TRef<Channel> channel)
{
}

void UDPClientReactor::onDisconnect(TRef<Channel> channel)
{
}

void UDPClientReactor::on_connect(uv_connect_t* req, int status)
{
}

void UDPClientReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
}

void UDPClientReactor::on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
}

void UDPClientReactor::on_stop(uv_async_t* handle)
{
}