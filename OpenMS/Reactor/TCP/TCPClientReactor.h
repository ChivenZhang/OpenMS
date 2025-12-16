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

class TCPClientReactor : public ChannelReactor
{
public:
	using callback_tcp_t = callback_t;

public:
	TCPClientReactor(MSRef<ISocketAddress> address, size_t workerNum, callback_tcp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;
	void onOutbound(MSRef<IChannelEvent> event, bool flush) override;

protected:
	MSRef<Channel> m_Channel;
	MSAtomic<bool> m_Sending;
	MSLambda<void()> m_FireAsync;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
};