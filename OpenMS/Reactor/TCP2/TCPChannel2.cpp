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
#include "TCPChannel2.h"
#include "../Private/ChannelReactor.h"

TCPChannel2::TCPChannel2(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID, asio::ip::tcp::socket handle)
	:
	Channel(reactor, local, remote, workID),
	m_Handle(std::move(handle)),
	m_RBuffer{}
{
}

asio::ip::tcp::socket* TCPChannel2::getHandle()
{
	return &m_Handle;
}

MSArrayView<char> TCPChannel2::getRBuffer()
{
	return m_RBuffer;
}
