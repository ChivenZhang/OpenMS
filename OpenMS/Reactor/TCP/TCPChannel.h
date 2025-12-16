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

class TCPChannel : public Channel
{
public:
	TCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, asio::ip::tcp::socket handle);
	asio::ip::tcp::socket* getSocket();
	MSArrayView<char> getBuffer();

protected:
	asio::ip::tcp::socket m_Socket;
	char m_Buffer[1024];
};