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
#include "TCPChannel.h"
#include "../Private/ChannelReactor.h"

TCPChannel::TCPChannel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, asio::ip::tcp::socket handle)
	:
	Channel(reactor, local, remote, workID),
	m_Socket(std::move(handle)),
	m_Buffer{}
{
}

asio::ip::tcp::socket* TCPChannel::getSocket()
{
	return &m_Socket;
}

MSArrayView<char> TCPChannel::getBuffer()
{
	return m_Buffer;
}
