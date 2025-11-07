#pragma once
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
#include "IChannelHandler.h"

/// @brief Interface for channel pipeline
class OPENMS_API IChannelPipeline
{
public:
	struct inbound_t
	{
		MSRef<IChannelInboundHandler> Handler;
		uint32_t HashName;
	};
	struct outbound_t
	{
		MSRef<IChannelOutboundHandler> Handler;
		uint32_t HashName;
	};
	struct handler_in
	{
		MSLambda<bool(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)> OnHandle;
		MSLambda<void(MSRaw<IChannelContext> context, MSError&& exception)> OnError;
	};
	struct handler_out
	{
		MSLambda<bool(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)> OnHandle;
		MSLambda<void(MSRaw<IChannelContext> context, MSError&& exception)> OnError;
	};

public:
	virtual ~IChannelPipeline() = default;

	virtual MSArrayView<const inbound_t> getInbounds() const = 0;

	virtual MSArrayView<const outbound_t> getOutbounds() const = 0;

	virtual bool addFirst(MSStringView name, MSRef<IChannelInboundHandler> handler) = 0;

	virtual bool addLast(MSStringView name, MSRef<IChannelInboundHandler> handler) = 0;

	virtual bool addBefore(MSStringView which, MSStringView name, MSRef<IChannelInboundHandler> handler) = 0;

	virtual bool addAfter(MSStringView which, MSStringView name, MSRef<IChannelInboundHandler> handler) = 0;

	virtual bool addFirst(MSStringView name, MSRef<IChannelOutboundHandler> handler) = 0;

	virtual bool addLast(MSStringView name, MSRef<IChannelOutboundHandler> handler) = 0;

	virtual bool addBefore(MSStringView which, MSStringView name, MSRef<IChannelOutboundHandler> handler) = 0;

	virtual bool addAfter(MSStringView which, MSStringView name, MSRef<IChannelOutboundHandler> handler) = 0;

	virtual bool addFirst(MSStringView name, handler_in handler) = 0;

	virtual bool addLast(MSStringView name, handler_in handler) = 0;

	virtual bool addBefore(MSStringView which, MSStringView name, handler_in handler) = 0;

	virtual bool addAfter(MSStringView which, MSStringView name, handler_in handler) = 0;

	virtual bool addFirst(MSStringView name, handler_out handler) = 0;

	virtual bool addLast(MSStringView name, handler_out handler) = 0;

	virtual bool addBefore(MSStringView which, MSStringView name, handler_out handler) = 0;

	virtual bool addAfter(MSStringView which, MSStringView name, handler_out handler) = 0;
};