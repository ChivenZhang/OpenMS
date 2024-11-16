/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "WSServerReactor.h"

WSServerReactor::WSServerReactor(TRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_t callback)
	:
	ChannelReactor(workerNum, callback)
{
}

void WSServerReactor::startup()
{
}

void WSServerReactor::shutdown()
{
}

void WSServerReactor::onConnect(TRef<Channel> channel)
{
}

void WSServerReactor::onDisconnect(TRef<Channel> channel)
{
}
