#include "ChannelContext.h"
#include "ChannelContext.h"
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "ChannelContext.h"
#include "Channel.h"

ChannelContext::ChannelContext(MSRaw<Channel> channel)
	:
	m_Channel(channel)
{
}

void ChannelContext::close()
{
	m_Channel->close();
}

MSFuture<bool> ChannelContext::close(MSPromise<bool>& promise)
{
	return m_Channel->close(promise);
}

void ChannelContext::write(MSRef<IChannelEvent> event)
{
	m_Channel->write(event);
}

MSFuture<bool> ChannelContext::write(MSRef<IChannelEvent> event, MSPromise<bool>& promise)
{
	return m_Channel->write(event, promise);
}

void ChannelContext::writeAndFlush(MSRef<IChannelEvent> event)
{
	m_Channel->writeAndFlush(event);
}

MSFuture<bool> ChannelContext::writeAndFlush(MSRef<IChannelEvent> event, MSPromise<bool>& promise)
{
	return m_Channel->writeAndFlush(event, promise);
}
