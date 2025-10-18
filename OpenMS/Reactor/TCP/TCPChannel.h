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
#include "../Private/Channel.h"
#include <uv.h>

class TCPChannel : public Channel
{
public:
	TCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, uv_tcp_t* handle);
	uv_tcp_t* getHandle() const;

protected:
	uv_tcp_t* m_Handle;
};