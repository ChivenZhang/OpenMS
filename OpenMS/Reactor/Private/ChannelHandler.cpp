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
#include "ChannelHandler.h"

bool ChannelInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	return false;
}

void ChannelInboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception)
{
	TError("%s", exception.what());
}

bool ChannelOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	return false;
}

void ChannelOutboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception)
{
	TError("%s", exception.what());
}

LambdaInboundHandler::LambdaInboundHandler(callback_t const& callback)
	:
	m_OnError(callback.OnError),
	m_OnRead(callback.OnRead)
{
}

bool LambdaInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	if (m_OnRead) return m_OnRead(context, event);
	return false;
}

void LambdaInboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception)
{
	if (m_OnError) m_OnError(context, std::forward<TException>(exception));
}

LambdaOutboundHandler::LambdaOutboundHandler(callback_t const& callback)
	:
	m_OnError(callback.OnError),
	m_OnWrite(callback.OnWrite)
{
}

bool LambdaOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)
{
	if (m_OnWrite) return m_OnWrite(context, event);
	return false;
}

void LambdaOutboundHandler::channelError(TRaw<IChannelContext> context, TException&& exception)
{
	if (m_OnError) m_OnError(context, std::forward<TException>(exception));
}