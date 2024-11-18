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

	using inconfig_t = IChannelInboundHandler::callback_t;

	using outconfig_t = IChannelOutboundHandler::callback_t;

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

	virtual bool addFirst(TStringView name, inconfig_t config) = 0;

	virtual bool addLast(TStringView name, inconfig_t config) = 0;

	virtual bool addBefore(TStringView which, TStringView name, inconfig_t config) = 0;

	virtual bool addAfter(TStringView which, TStringView name, inconfig_t config) = 0;

	virtual bool addFirst(TStringView name, outconfig_t config) = 0;

	virtual bool addLast(TStringView name, outconfig_t config) = 0;

	virtual bool addBefore(TStringView which, TStringView name, outconfig_t config) = 0;

	virtual bool addAfter(TStringView which, TStringView name, outconfig_t config) = 0;
};