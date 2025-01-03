#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
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
		MSRef<IChannelInboundHandler> Handler;
		uint32_t HashName;
	};

	struct outbound_t
	{
		MSRef<IChannelOutboundHandler> Handler;
		uint32_t HashName;
	};

	using inconfig_t = IChannelInboundHandler::callback_t;

	using outconfig_t = IChannelOutboundHandler::callback_t;

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

	virtual bool addFirst(MSStringView name, inconfig_t config) = 0;

	virtual bool addLast(MSStringView name, inconfig_t config) = 0;

	virtual bool addBefore(MSStringView which, MSStringView name, inconfig_t config) = 0;

	virtual bool addAfter(MSStringView which, MSStringView name, inconfig_t config) = 0;

	virtual bool addFirst(MSStringView name, outconfig_t config) = 0;

	virtual bool addLast(MSStringView name, outconfig_t config) = 0;

	virtual bool addBefore(MSStringView which, MSStringView name, outconfig_t config) = 0;

	virtual bool addAfter(MSStringView which, MSStringView name, outconfig_t config) = 0;
};