#pragma once
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
#include "../Private/ChannelReactor.h"
#include "../Private/ChannelAddress.h"
#include <uv.h>

class WSServerReactor : public ChannelReactor
{
public:
	WSServerReactor(TRef<ISocketAddress> address, uint32_t backlog, size_t workerNum, callback_t callback);
	void startup() override;
	void shutdown() override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onDisconnect(TRef<Channel> channel) override;

protected:
	TRef<ISocketAddress> m_Address;
	TMap<uint32_t, TRef<Channel>> m_ChannelMap;
};