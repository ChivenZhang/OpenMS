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
#include "../IChannel.h"
#include "ChannelContext.h"
#include "ChannelPipeline.h"
class ChannelReactor;

class Channel : public IChannel, public std::enable_shared_from_this<Channel>
{
public:
	Channel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID);
	~Channel();
	bool running() const override;
	uint32_t getWorkID() const override;
	MSHnd<IChannelAddress> getLocal() const override;
	MSHnd<IChannelAddress> getRemote() const override;
	MSRaw<IChannelContext> getContext() const override;
	MSRaw<IChannelPipeline> getPipeline() const override;
	void readChannel(MSRef<IChannelEvent> event) override;
	void writeChannel(MSRef<IChannelEvent> event) override;
	void close() override;
	MSFuture<bool> close(MSPromise<bool>& promise) override;
	void write(MSRef<IChannelEvent> event) override;
	MSFuture<bool> write(MSRef<IChannelEvent> event, MSPromise<bool>& promise) override;
	void writeAndFlush(MSRef<IChannelEvent> event) override;
	MSFuture<bool> writeAndFlush(MSRef<IChannelEvent> event, MSPromise<bool>& promise) override;

protected:
	const uint32_t m_WorkID;
	MSAtomic<bool> m_Running;
	ChannelContext m_Context;
	ChannelPipeline m_Pipeline;
	MSRaw<ChannelReactor> m_Reactor;
	MSRef<IChannelAddress> m_LocalAddr;
	MSRef<IChannelAddress> m_RemoteAddr;
};