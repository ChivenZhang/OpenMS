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
#include "../IChannelHandler.h"

class ChannelInboundHandler : public IChannelInboundHandler
{
public:
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;
	void channelError(MSRaw<IChannelContext> context, MSError&& exception) override;
};

class ChannelOutboundHandler : public IChannelOutboundHandler
{
public:
	bool channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;
	void channelError(MSRaw<IChannelContext> context, MSError&& exception) override;
};

class LambdaInboundHandler : public ChannelInboundHandler
{
public:
	struct callback_t
	{
		MSLambda<bool(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)> OnRead;
		MSLambda<void(MSRaw<IChannelContext> context, MSError&& exception)> OnError;
	};

public:
	LambdaInboundHandler(callback_t const& callback);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;
	void channelError(MSRaw<IChannelContext> context, MSError&& exception) override;

protected:
	MSLambda<bool(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)> m_OnRead;
	MSLambda<void(MSRaw<IChannelContext> context, MSError&& exception)> m_OnError;
};

class LambdaOutboundHandler : public ChannelOutboundHandler
{
public:
	struct callback_t
	{
		MSLambda<bool(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)> OnWrite;
		MSLambda<void(MSRaw<IChannelContext> context, MSError&& exception)> OnError;
	};

public:
	LambdaOutboundHandler(callback_t const& callback);
	bool channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;
	void channelError(MSRaw<IChannelContext> context, MSError&& exception) override;

protected:
	MSLambda<bool(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)> m_OnWrite;
	MSLambda<void(MSRaw<IChannelContext> context, MSError&& exception)> m_OnError;
};