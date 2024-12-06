#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "../IChannelContext.h"
class Channel;

class ChannelContext : public IChannelContext
{
public:
	ChannelContext(MSRaw<Channel> channel);
	void close() override;
	MSFuture<bool> close(MSPromise<bool>& promise) override;
	void write(MSRef<IChannelEvent> event) override;
	MSFuture<bool> write(MSRef<IChannelEvent> event, MSPromise<bool>& promise) override;
	void writeAndFlush(MSRef<IChannelEvent> event) override;
	MSFuture<bool> writeAndFlush(MSRef<IChannelEvent> event, MSPromise<bool>& promise) override;

protected:
	MSRaw<Channel> m_Channel;
};