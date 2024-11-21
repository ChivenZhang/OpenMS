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

class TCPChannel : public Channel
{
public:
	TCPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uint32_t workid, uv_tcp_t* handle);
	~TCPChannel();
	uv_tcp_t* getHandle() const;

protected:
	uv_tcp_t* m_Handle;
};