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
#include "TCPClientReactor.h"

TCPClientReactor::TCPClientReactor(TStringView ip, uint16_t port, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback),
	m_Address(ip),
	m_PortNum(port),
	m_AsyncStop(uv_async_t())
{
}

void TCPClientReactor::startup()
{
	if (m_Running == true) return;
	ChannelReactor::startup();
}

void TCPClientReactor::shutdown()
{
	if (m_Running == false) return;
	ChannelReactor::shutdown();
}

void TCPClientReactor::onConnect(TRef<Channel> channel)
{
}

void TCPClientReactor::onDisconnect(TRef<Channel> channel)
{
}

void TCPClientReactor::on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
}

void TCPClientReactor::on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
}

void TCPClientReactor::on_write(uv_write_t* req, int status)
{
}

void TCPClientReactor::on_stop(uv_async_t* handle)
{
}
