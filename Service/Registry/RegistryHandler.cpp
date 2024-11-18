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
#include "RegistryHandler.h"

bool RegistryInboundHandler::channelRead(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	return false;
}

bool RegistryOutboundHandler::channelWrite(TRaw<IChannelContext> context, TRaw<IChannelEvent> event) const
{
	return false;
}
