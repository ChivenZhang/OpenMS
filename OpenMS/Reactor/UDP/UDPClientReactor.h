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

class UDPClientReactor : public ChannelReactor
{
public:
	using callback_udp_t = callback_t;

public:
	UDPClientReactor(MSRef<ISocketAddress> address, bool broadcast, bool multicast, size_t workerNum, callback_udp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) override;
	void writeAndFlush(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;

protected:
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
	static void on_send(uv_timer_t* handle);

protected:
	bool m_Broadcast, m_Multicast;
	MSRef<Channel> m_Channel;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
	MSMap<MSRaw<IChannelEvent>, MSRef<IChannelEvent>> m_EventCache;
};