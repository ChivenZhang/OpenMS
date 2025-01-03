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

class KCPServerReactor : public ChannelReactor
{
public:
	struct callback_kcp_t : public callback_t
	{
		MSLambda<uint32_t(MSRef<IChannelAddress>)> Session;
	};

public:
	KCPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_kcp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;

protected:
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
	static int on_output(const char* buf, int len, struct IKCPCB* kcp, void* user);
	static void on_send(uv_udp_t* handle);

protected:
	uint32_t m_Backlog, m_Session;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
	MSList<MSRef<Channel>> m_Channels;
	MSList<MSRef<Channel>> m_ChannelsRemoved;
	MSMap<uint32_t, MSRef<Channel>> m_ChannelMap;
	MSLambda<uint32_t(MSRef<IChannelAddress>)> m_OnSession;
};