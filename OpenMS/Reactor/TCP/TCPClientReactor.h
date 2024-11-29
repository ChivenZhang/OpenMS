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

class TCPClientReactor : public ChannelReactor
{
public:
	using callback_tcp_t = callback_t;

public:
	TCPClientReactor(TRef<ISocketAddress> address, size_t workerNum, callback_tcp_t callback);
	void startup() override;
	void shutdown() override;
	THnd<IChannelAddress> address() const override;
	void write(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;
	void writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onDisconnect(TRef<Channel> channel) override;

protected:
	static void on_connect(uv_connect_t* req, int status);
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void on_send(uv_tcp_t* handle);

protected:
	TRef<Channel> m_Channel;
	TRef<ISocketAddress> m_Address;
	TRef<ISocketAddress> m_LocalAddress;
};