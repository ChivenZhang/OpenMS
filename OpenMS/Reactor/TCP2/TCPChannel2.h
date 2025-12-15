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

class TCPChannel2 : public Channel
{
public:
	TCPChannel2(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, asio::ip::tcp::socket handle);
	asio::ip::tcp::socket* getHandle();
	MSArrayView<char> getRBuffer();

protected:
	asio::ip::tcp::socket m_Handle;
	char m_RBuffer[1024];
};