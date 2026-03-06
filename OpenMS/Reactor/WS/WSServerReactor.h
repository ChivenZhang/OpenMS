#pragma once
/*=================================================
* Copyright © 2020-2026 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../Private/ChannelReactor.h"
#include <websocketpp/common/connection_hdl.hpp>

class WSServerReactor : public ChannelReactor
{
public:
	WSServerReactor(MSRef<IWebSocketAddress> address, uint32_t backlog, size_t workerNum, const callback_t& callback);
	void startup() override;
	void shutdown() override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event) override;

protected:
	void onConnect(MSRef<Channel> channel) override;
	void onDisconnect(MSRef<Channel> channel) override;
	void onOutbound(MSRef<IChannelEvent> event, bool flush) override;

protected:
	const uint32_t m_Backlog;
	MSRef<IWebSocketAddress> m_Address;
	MSRef<IWebSocketAddress> m_LocalAddress;
	MSLambda<void(MSRef<IChannelEvent>)> m_FireAsync;
	MSMap<websocketpp::connection_hdl::element_type*, MSRef<Channel>> m_ChannelMap;
};