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
#include "ChannelHandler.h"

void ChannelInboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception) const
{
}

bool ChannelInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	return false;
}

void ChannelOutboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception) const
{
}

bool ChannelOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	return false;
}

LambdaInboundHandler::LambdaInboundHandler(callback_t const& callback)
	:
	m_OnError(callback.OnError),
	m_OnRead(callback.OnRead)
{
}

void LambdaInboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception) const
{
	if (m_OnError) m_OnError(context, std::forward<TException>(exception));
}

bool LambdaInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	if (m_OnRead) return m_OnRead(context, event);
	return false;
}

LambdaOutboundHandler::LambdaOutboundHandler(callback_t const& callback)
	:
	m_OnError(callback.OnError),
	m_OnWrite(callback.OnWrite)
{
}

void LambdaOutboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception) const
{
	if (m_OnError) m_OnError(context, std::forward<TException>(exception));
}

bool LambdaOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	if (m_OnWrite) return m_OnWrite(context, event);
	return false;
}
