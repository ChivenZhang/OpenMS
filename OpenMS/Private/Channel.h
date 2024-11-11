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
#include "IChannel.h"
#include "ChannelContext.h"
#include "ChannelPipeline.h"
class ChannelReactor;

class Channel : public IChannel
{
public:
	Channel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote);
	~Channel();
	bool running() const override;
	TRaw<const IChannelAddress> getLocal() const override;
	TRaw<const IChannelAddress> getRemote() const override;
	TRaw<const IChannelContext> getContext() const override;
	TRaw<IChannelPipeline> getPipeline() override;
	TRaw<const IChannelPipeline> getPipeline() const override;
	void close() override;
	TFuture<bool> close(TPromise<bool>&& promise) override;
	void read(TRaw<IChannelEvent> event) override;
	TFuture<bool> read(TRaw<IChannelEvent> event, TPromise<bool>&& promise) override;
	void write(TRaw<IChannelEvent> event) override;
	TFuture<bool> write(TRaw<IChannelEvent> event, TPromise<bool>&& promise) override;

protected:
	TAtomic<bool> m_Running;
	ChannelContext m_Context;
	ChannelPipeline m_Pipeline;
	TRaw<ChannelReactor> m_Reactor;
	TRef<IChannelAddress> m_LocalAddr;
	TRef<IChannelAddress> m_RemoteAddr;
};