#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
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
	TCPClientReactor(MSRef<ISocketAddress> address, size_t workerNum, callback_tcp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) override;
	void writeAndFlush(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;

protected:
	static void on_connect(uv_connect_t* req, int status);
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void on_send(uv_timer_t* handle);

protected:
	MSRef<Channel> m_Channel;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
	MSMap<MSRaw<IChannelEvent>, MSRef<IChannelEvent>> m_EventCache;
};