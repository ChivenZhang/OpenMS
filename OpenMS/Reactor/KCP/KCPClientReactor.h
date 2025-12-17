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

class KCPClientReactor : public ChannelReactor
{
public:
	struct callback_kcp_t : callback_t
	{
		MSLambda<uint32_t(MSRef<IChannelAddress>)> Session;
	};

public:
	KCPClientReactor(MSRef<ISocketAddress> address, size_t workerNum, callback_kcp_t callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;
	void onOutbound(MSRef<IChannelEvent> event, bool flush) override;
	static int on_output(const char* buf, int len, struct IKCPCB* kcp, void* user);

protected:
	MSRef<Channel> m_Channel;
	MSRef<Channel> m_ChannelRemoved;
	MSRef<ISocketAddress> m_Address;
	MSRef<ISocketAddress> m_LocalAddress;
};