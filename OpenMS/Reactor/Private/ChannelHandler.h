#pragma once
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
#include "../IChannelHandler.h"

class ChannelInboundHandler : public IChannelInboundHandler
{
public:
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};

class ChannelOutboundHandler : public IChannelOutboundHandler
{
public:
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;
};

class LambdaInboundHandler : public ChannelInboundHandler
{
public:
	struct callback_t
	{
		TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> OnRead;
		TLambda<void(TRaw<IChannelContext> context, TException&& exception)> OnError;
	};

public:
	LambdaInboundHandler(callback_t const& callback);
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;

protected:
	TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> m_OnRead;
	TLambda<void(TRaw<IChannelContext> context, TException&& exception)> m_OnError;
};

class LambdaOutboundHandler : public ChannelOutboundHandler
{
public:
	struct callback_t
	{
		TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> OnWrite;
		TLambda<void(TRaw<IChannelContext> context, TException&& exception)> OnError;
	};

public:
	LambdaOutboundHandler(callback_t const& callback);
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;

protected:
	TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> m_OnWrite;
	TLambda<void(TRaw<IChannelContext> context, TException&& exception)> m_OnError;
};