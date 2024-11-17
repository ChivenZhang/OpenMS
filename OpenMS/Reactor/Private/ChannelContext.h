#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../IChannelContext.h"
class Channel;

class ChannelContext : public IChannelContext
{
public:
	ChannelContext(TRaw<Channel> channel);
	void close() override;
	TFuture<bool> close(TPromise<bool>&& promise) override;
	void write(TRef<IChannelEvent> event) override;
	TFuture<bool> write(TRef<IChannelEvent> event, TPromise<bool>&& promise) override;
	void writeAndFlush(TRef<IChannelEvent> event) override;
	TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TPromise<bool>&& promise) override;

protected:
	TRaw<Channel> m_Channel;
};