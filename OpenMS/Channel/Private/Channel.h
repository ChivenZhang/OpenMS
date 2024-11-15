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
#include "../IChannel.h"
#include "ChannelContext.h"
#include "ChannelPipeline.h"
class ChannelReactor;

class Channel : public IChannel, public std::enable_shared_from_this<Channel>
{
public:
	Channel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote);
	~Channel();
	bool running() const override;
	TRaw<IChannelAddress> getLocal() const override;
	TRaw<IChannelAddress> getRemote() const override;
	TRaw<IChannelContext> getContext() const override;
	TRaw<IChannelPipeline> getPipeline() const override;
	void close() override;
	TFuture<bool> close(TPromise<bool>&& promise) override;
	void read(TRef<IChannelEvent> event);
	TFuture<bool> read(TRef<IChannelEvent> event, TPromise<bool>&& promise);
	void write(TRef<IChannelEvent> event) override;
	TFuture<bool> write(TRef<IChannelEvent> event, TPromise<bool>&& promise) override;
	void writeAndFlush(TRef<IChannelEvent> event) override;
	TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TPromise<bool>&& promise) override;

protected:
	TAtomic<bool> m_Running;
	ChannelContext m_Context;
	ChannelPipeline m_Pipeline;
	TRaw<ChannelReactor> m_Reactor;
	TRef<IChannelAddress> m_LocalAddr;
	TRef<IChannelAddress> m_RemoteAddr;
};