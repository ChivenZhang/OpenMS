#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../IEndpoint.h"
#include "../Reactor/TCP/TCPServerReactor.h"

class TCPServer : public IEndpoint
{
public:

protected:
	TRef<ISocketAddress> m_Address;
};