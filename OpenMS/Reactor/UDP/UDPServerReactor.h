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

class UDPServerReactor : public ChannelReactor
{
public:
	using callback_udp_t = callback_t;

public:
	UDPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, bool broadcast, bool multicast, size_t workerNum, callback_udp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;
	void onOutbound(MSRef<IChannelEvent> event, bool flush) override;

protected:
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
	static void on_send(uv_async_t* handle);

protected:
	uint32_t m_Backlog;
	bool m_Broadcast, m_Multicast;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
	MSList<MSRef<Channel>> m_Channels;
	MSMap<uint32_t, MSHnd<Channel>> m_ChannelMap;
	MSMap<MSRaw<IChannelEvent>, MSRef<IChannelEvent>> m_EventCache;
	MSAtomic<bool> m_Sending;
	uv_async_t* m_EventAsync = nullptr;
};