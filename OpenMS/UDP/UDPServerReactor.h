#pragma once
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
#include "../Private/ChannelReactor.h"
#include "../Private/ChannelAddress.h"
#include <uv.h>

class UDPServerReactor : public ChannelReactor
{
public:
	UDPServerReactor(TRef<ISocketAddress> address, bool broadcast, bool multicast, size_t workerNum, callback_t callback);
	void startup() override;
	void shutdown() override;
	void write(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;
	void writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onDisconnect(TRef<Channel> channel) override;

protected:
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
	static void on_send(uv_udp_t* handle);
	static void on_stop(uv_async_t* handle);

protected:
	uv_async_t m_AsyncStop;
	bool m_Broadcast, m_Multicast;
	TRef<ISocketAddress> m_SocketAddress;
	TMap<uint32_t, TRef<Channel>> m_Connections;
};