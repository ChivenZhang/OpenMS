#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../Private/ChannelReactor.h"
#include "../Private/ChannelAddress.h"
#include <uv.h>

class TCPServerReactor : public ChannelReactor
{
public:
	TCPServerReactor(TRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_t callback);
	void startup() override;
	void shutdown() override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onDisconnect(TRef<Channel> channel) override;

protected:
	static void on_connect(uv_stream_t* server, int status);
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void on_send(uv_tcp_t* handle);
	static void on_stop(uv_async_t* handle);

protected:
	uint32_t m_Backlog;
	uv_async_t m_AsyncStop;
	TRef<ISocketAddress> m_Address;
	TMap<uint32_t, TRef<Channel>> m_ChannelMap;
};