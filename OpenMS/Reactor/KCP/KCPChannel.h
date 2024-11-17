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
#include "../../External/kcp/ikcp.h"

class KCPChannel : public Channel
{
public:
	KCPChannel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uint32_t workID, uv_udp_t* handle, ikcpcb* session);
	~KCPChannel();
	uv_udp_t* getHandle() const;
	ikcpcb* getSession() const;
	TRaw<ChannelReactor> getReactor() const;

protected:
	uv_udp_t* m_Handle;
	ikcpcb* m_Session;
};