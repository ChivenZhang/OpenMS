#include "ChannelContext.h"
#include "ChannelContext.h"
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

MSStringMap<MSAny> &ChannelContext::attrib()
{
	return m_AttribMap;
}
