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
#include "../IChannel.h"
#include "ChannelContext.h"
#include "ChannelPipeline.h"
class ChannelReactor;

class Channel : public IChannel, public std::enable_shared_from_this<Channel>
{
public:
	Channel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uint32_t workID);
	~Channel();
	bool running() const override;
	uint32_t getWorkID() const override;
	TRaw<IChannelAddress> getLocal() const override;
	TRaw<IChannelAddress> getRemote() const override;
	TRaw<IChannelContext> getContext() const override;
	TRaw<IChannelPipeline> getPipeline() const override;
	void close() override;
	TFuture<bool> close(TPromise<bool>& promise) override;
	void read(TRef<IChannelEvent> event);
	TFuture<bool> read(TRef<IChannelEvent> event, TPromise<bool>& promise);
	void write(TRef<IChannelEvent> event) override;
	TFuture<bool> write(TRef<IChannelEvent> event, TPromise<bool>& promise) override;
	void writeAndFlush(TRef<IChannelEvent> event) override;
	TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TPromise<bool>& promise) override;

protected:
	const uint32_t m_WorkID;
	TAtomic<bool> m_Running;
	ChannelContext m_Context;
	ChannelPipeline m_Pipeline;
	TRaw<ChannelReactor> m_Reactor;
	TRef<IChannelAddress> m_LocalAddr;
	TRef<IChannelAddress> m_RemoteAddr;
};