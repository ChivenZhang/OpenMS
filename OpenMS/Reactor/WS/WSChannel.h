#pragma once
/*=================================================
* Copyright © 2020-2026 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../Private/Channel.h"
#include <websocketpp/connection.hpp>

class WSChannel : public Channel
{
public:
	WSChannel(MSRaw<ChannelReactor> reactor, const MSRef<IChannelAddress>& local, const MSRef<IChannelAddress>& remote, uint32_t workID, websocketpp::connection_hdl handle)
		:
		Channel(reactor, local, remote, workID),
		m_Handle(handle)
	{
	}
	websocketpp::connection_hdl getHandle() const
	{
		return m_Handle;
	}

protected:
	websocketpp::connection_hdl m_Handle;
};