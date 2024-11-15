#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "../IChannelPipeline.h"
class Channel;

class ChannelPipeline : public IChannelPipeline
{
public:
	ChannelPipeline(TRaw<Channel> channel);
	TArrayView<const inbound_t> getInbounds() const override;
	TArrayView<const outbound_t> getOutbounds() const override;
	bool addFirst(TStringView name, TRef<IChannelInboundHandler> handler) override;
	bool addLast(TStringView name, TRef<IChannelInboundHandler> handler) override;
	bool addBefore(TStringView which, TStringView name, TRef<IChannelInboundHandler> handler) override;
	bool addAfter(TStringView which, TStringView name, TRef<IChannelInboundHandler> handler) override;
	bool addFirst(TStringView name, TRef<IChannelOutboundHandler> handler) override;
	bool addLast(TStringView name, TRef<IChannelOutboundHandler> handler) override;
	bool addBefore(TStringView which, TStringView name, TRef<IChannelOutboundHandler> handler) override;
	bool addAfter(TStringView which, TStringView name, TRef<IChannelOutboundHandler> handler) override;

protected:
	TRaw<Channel> m_Channel;
	TVector<inbound_t> m_Inbounds;
	TVector<outbound_t> m_Outbounds;
};