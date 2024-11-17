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
#include "../Private/Channel.h"
#include <uv.h>

class UDPChannel : public Channel
{
public:
	UDPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uint32_t workID, uv_udp_t* handle);
	uv_udp_t* getHandle() const;

protected:
	uv_udp_t* m_Handle;
};