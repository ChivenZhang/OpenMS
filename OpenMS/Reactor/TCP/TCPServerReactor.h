#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
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
	TCPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_tcp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;

protected:
	static void on_connect(uv_stream_t* server, int status);
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void on_send(uv_timer_t* handle);

protected:
	uint32_t m_Backlog;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
	MSMap<uint32_t, MSRef<Channel>> m_ChannelMap;
	MSMap<MSRaw<IChannelEvent>, MSRef<IChannelEvent>> m_EventCache;
};