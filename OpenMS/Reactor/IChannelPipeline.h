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
#include "MS.h"
#include "IChannelHandler.h"

/// @brief Interface for channel pipeline
class OPENMS_API IChannelPipeline
{
public:
	struct inbound_t
	{
		TRef<IChannelInboundHandler> Handler;
		uint32_t HashName;
	};

	struct outbound_t
	{
		TRef<IChannelOutboundHandler> Handler;
		uint32_t HashName;
	};

public:
	virtual ~IChannelPipeline() = default;

	virtual TArrayView<const inbound_t> getInbounds() const = 0;

	virtual TArrayView<const outbound_t> getOutbounds() const = 0;

	virtual bool addFirst(TStringView name, TRef<IChannelInboundHandler> handler) = 0;

	virtual bool addLast(TStringView name, TRef<IChannelInboundHandler> handler) = 0;

	virtual bool addBefore(TStringView which, TStringView name, TRef<IChannelInboundHandler> handler) = 0;

	virtual bool addAfter(TStringView which, TStringView name, TRef<IChannelInboundHandler> handler) = 0;

	virtual bool addFirst(TStringView name, TRef<IChannelOutboundHandler> handler) = 0;

	virtual bool addLast(TStringView name, TRef<IChannelOutboundHandler> handler) = 0;

	virtual bool addBefore(TStringView which, TStringView name, TRef<IChannelOutboundHandler> handler) = 0;

	virtual bool addAfter(TStringView which, TStringView name, TRef<IChannelOutboundHandler> handler) = 0;
};