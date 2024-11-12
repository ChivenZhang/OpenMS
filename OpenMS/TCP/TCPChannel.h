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

class TCPChannel : public Channel
{
public:
	TCPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uv_tcp_t* handle);
	uv_tcp_t* getHandle() const;

protected:
	uv_tcp_t* m_Handle;
};