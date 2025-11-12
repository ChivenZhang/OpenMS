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

class KCPClientReactor : public ChannelReactor
{
public:
	using callback_kcp_t = callback_t;

public:
	KCPClientReactor(MSRef<ISocketAddress> address, size_t workerNum, callback_kcp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;
	void onOutbound(MSRef<IChannelEvent> event, bool flush) override;

protected:
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
	static int on_output(const char* buf, int len, struct IKCPCB* kcp, void* user);
	static void on_send(uv_async_t* handle);

protected:
	MSRef<Channel> m_Channel;
	MSRef<Channel> m_ChannelRemoved;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
	uv_async_t* m_EventAsync = nullptr;
};