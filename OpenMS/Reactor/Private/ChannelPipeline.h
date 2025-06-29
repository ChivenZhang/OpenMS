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
#include "../IChannelPipeline.h"
class Channel;

class ChannelPipeline : public IChannelPipeline
{
public:
	ChannelPipeline(MSRaw<Channel> channel);
	MSArrayView<const inbound_t> getInbounds() const override;
	MSArrayView<const outbound_t> getOutbounds() const override;
	bool addFirst(MSStringView name, MSRef<IChannelInboundHandler> handler) override;
	bool addLast(MSStringView name, MSRef<IChannelInboundHandler> handler) override;
	bool addBefore(MSStringView which, MSStringView name, MSRef<IChannelInboundHandler> handler) override;
	bool addAfter(MSStringView which, MSStringView name, MSRef<IChannelInboundHandler> handler) override;
	bool addFirst(MSStringView name, MSRef<IChannelOutboundHandler> handler) override;
	bool addLast(MSStringView name, MSRef<IChannelOutboundHandler> handler) override;
	bool addBefore(MSStringView which, MSStringView name, MSRef<IChannelOutboundHandler> handler) override;
	bool addAfter(MSStringView which, MSStringView name, MSRef<IChannelOutboundHandler> handler) override;
	bool addFirst(MSStringView name, handler_in handler) override;
	bool addLast(MSStringView name, handler_in handler) override;
	bool addBefore(MSStringView which, MSStringView name, handler_in handler) override;
	bool addAfter(MSStringView which, MSStringView name, handler_in handler) override;
	bool addFirst(MSStringView name, handler_out handler) override;
	bool addLast(MSStringView name, handler_out handler) override;
	bool addBefore(MSStringView which, MSStringView name, handler_out handler) override;
	bool addAfter(MSStringView which, MSStringView name, handler_out handler) override;

protected:
	MSRaw<Channel> m_Channel;
	MSList<inbound_t> m_Inbounds;
	MSList<outbound_t> m_Outbounds;
};