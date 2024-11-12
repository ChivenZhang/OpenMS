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
#include "../Private/Channel.h"
#include <uv.h>

class UDPChannel : public Channel
{
public:
	UDPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uv_udp_t* handle);
	uv_udp_t* getHandle() const;

protected:
	uv_udp_t* m_Handle;
};