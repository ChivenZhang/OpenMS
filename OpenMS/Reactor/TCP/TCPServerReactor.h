#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "../Private/ChannelReactor.h"
#include "../Private/ChannelAddress.h"
#include <uv.h>

class TCPServerReactor : public ChannelReactor
{
public:
	using callback_tcp_t = callback_t;

public:
	TCPServerReactor(TRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_tcp_t callback);
	void startup() override;
	void shutdown() override;
	THnd<IChannelAddress> address() const override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onOnClose(TRef<Channel> channel) override;

protected:
	static void on_connect(uv_stream_t* server, int status);
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void on_send(uv_tcp_t* handle);

protected:
	uint32_t m_Backlog;
	TRef<ISocketAddress> m_Address;
	TRef<ISocketAddress> m_LocalAddress;
	TMap<uint32_t, TRef<Channel>> m_ChannelMap;
};