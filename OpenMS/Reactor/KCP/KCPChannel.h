#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "../Private/Channel.h"
#include <uv.h>
#include <ikcp.h>

class KCPChannel : public Channel
{
public:
	KCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, uv_udp_t* handle, ikcpcb* session);
	~KCPChannel();
	uv_udp_t* getHandle() const;
	ikcpcb* getSession() const;
	MSRaw<ChannelReactor> getReactor() const;

protected:
	uv_udp_t* m_Handle;
	ikcpcb* m_Session;
};