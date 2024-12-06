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
#include "ChannelHandler.h"

bool ChannelInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	return false;
}

void ChannelInboundHandler::channelError(MSRaw<IChannelContext> context, MSError&& exception)
{
	MSError("%s", exception.what());
}

bool ChannelOutboundHandler::channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	return false;
}

void ChannelOutboundHandler::channelError(MSRaw<IChannelContext> context, MSError&& exception)
{
	MSError("%s", exception.what());
}

LambdaInboundHandler::LambdaInboundHandler(callback_t const& callback)
	:
	m_OnError(callback.OnError),
	m_OnRead(callback.OnRead)
{
}

bool LambdaInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	if (m_OnRead) return m_OnRead(context, event);
	return false;
}

void LambdaInboundHandler::channelError(MSRaw<IChannelContext> context, MSError&& exception)
{
	if (m_OnError) m_OnError(context, std::forward<MSError>(exception));
}

LambdaOutboundHandler::LambdaOutboundHandler(callback_t const& callback)
	:
	m_OnError(callback.OnError),
	m_OnWrite(callback.OnWrite)
{
}

bool LambdaOutboundHandler::channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	if (m_OnWrite) return m_OnWrite(context, event);
	return false;
}

void LambdaOutboundHandler::channelError(MSRaw<IChannelContext> context, MSError&& exception)
{
	if (m_OnError) m_OnError(context, std::forward<MSError>(exception));
}