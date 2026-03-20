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
#include "OpenMS/Reactor/Private/ChannelHandler.h"

class AESInboundHandler : public ChannelInboundHandler
{
public:
	struct config_t
	{
		MSArray<uint8_t, 32> Key;
		MSArray<uint8_t, 16> IV;
	};

public:
	explicit AESInboundHandler(config_t const& config);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	config_t m_Config;
};

class AESOutboundHandler : public ChannelOutboundHandler
{
public:
	struct config_t
	{
		MSArray<uint8_t, 32> Key;
		MSArray<uint8_t, 16> IV;
	};

public:
	explicit AESOutboundHandler(config_t const& config);
	bool channelWrite(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	config_t m_Config;
};