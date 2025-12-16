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
#include <asio.hpp>

class UDPChannel : public Channel
{
public:
	UDPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, asio::ip::udp::endpoint handle);
	asio::ip::udp::endpoint const& getEndpoint() const;

protected:
	asio::ip::udp::endpoint m_Endpoint;
};