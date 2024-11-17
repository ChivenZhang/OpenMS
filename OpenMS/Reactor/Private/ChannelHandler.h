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
		TLambda<void(TRaw<IChannelContext> context, TException&& exception)> OnError;
		TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> OnRead;
	};

public:
	LambdaInboundHandler(callback_t const& callback);
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;

protected:
	TLambda<void(TRaw<IChannelContext> context, TException&& exception)> m_OnError;
	TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> m_OnRead;
};

class LambdaOutboundHandler : public ChannelOutboundHandler
{
public:
	struct callback_t
	{
		TLambda<void(TRaw<IChannelContext> context, TException&& exception)> OnError;
		TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> OnWrite;
	};

public:
	LambdaOutboundHandler(callback_t const& callback);
	void channelError(TRaw<IChannelContext> context, TException&& exception) const override;
	bool channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const override;

protected:
	TLambda<void(TRaw<IChannelContext> context, TException&& exception)> m_OnError;
	TLambda<bool(TRaw<IChannelContext> context, TRaw<IChannelEvent> event)> m_OnWrite;
};