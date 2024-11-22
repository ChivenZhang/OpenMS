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

class KCPServerReactor : public ChannelReactor
{
public:
	struct callback_kcp_t : public callback_t
	{
		TLambda<uint32_t(TRef<IChannelAddress>)> Session;
	};

public:
	KCPServerReactor(TRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_kcp_t callback);
	void startup() override;
	void shutdown() override;
	THnd<IChannelAddress> address() const override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onDisconnect(TRef<Channel> channel) override;

protected:
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_udp_t* req, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
	static int on_output(const char* buf, int len, struct IKCPCB* kcp, void* user);
	static void on_send(uv_udp_t* handle);

protected:
	uint32_t m_Backlog, m_Session;
	TRef<ISocketAddress> m_Address;
	TRef<ISocketAddress> m_LocalAddress;
	TVector<TRef<Channel>> m_Channels;
	TVector<TRef<Channel>> m_ChannelsRemoved;
	TMap<uint32_t, TRef<Channel>> m_ChannelMap;
	TLambda<uint32_t(TRef<IChannelAddress>)> m_OnSession;
};