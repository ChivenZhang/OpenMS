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

class TCPServerReactor : public ChannelReactor
{
public:
	using callback_tcp_t = callback_t;

public:
	TCPServerReactor(MSRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_tcp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;
	void onOutbound(MSRef<IChannelEvent> event, bool flush) override;

protected:
	uint32_t m_Backlog;
	MSAtomic<bool> m_Sending;
	MSLambda<void()> m_FireAsync;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
	MSMap<uint32_t, MSRef<Channel>> m_ChannelMap;
};