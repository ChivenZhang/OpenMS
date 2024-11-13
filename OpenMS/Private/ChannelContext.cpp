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
#include "ChannelContext.h"
#include "Channel.h"

ChannelContext::ChannelContext(TRaw<Channel> channel)
	:
	m_Channel(channel)
{
}

void ChannelContext::write(TRef<IChannelEvent> event)
{
	m_Channel->write(event);
}

void ChannelContext::writeAndFlush(TRef<IChannelEvent> event)
{
	m_Channel->writeAndFlush(event);
}

TFuture<bool> ChannelContext::write(TRef<IChannelEvent> event, TPromise<bool>&& promise)
{
	return m_Channel->write(event, std::forward<TPromise<bool>>(promise));
}

TFuture<bool> ChannelContext::writeAndFlush(TRef<IChannelEvent> event, TPromise<bool>&& promise)
{
	return m_Channel->writeAndFlush(event, std::forward<TPromise<bool>>(promise));
}
