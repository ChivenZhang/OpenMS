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

class KCPClientReactor : public ChannelReactor
{
public:
	using callback_kcp_t = callback_t;

public:
	KCPClientReactor(TRef<ISocketAddress> address, size_t workerNum, callback_kcp_t callback);
	void startup() override;
	void shutdown() override;
	THnd<IChannelAddress> address() const override;
	void write(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;
	void writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onDisconnect(TRef<Channel> channel) override;

protected:
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
	static int on_output(const char* buf, int len, struct IKCPCB* kcp, void* user);
	static void on_send(uv_udp_t* handle);

protected:
	TRef<Channel> m_Channel;
	TRef<Channel> m_ChannelRemoved;
	TRef<ISocketAddress> m_Address;
	TRef<ISocketAddress> m_LocalAddress;
};