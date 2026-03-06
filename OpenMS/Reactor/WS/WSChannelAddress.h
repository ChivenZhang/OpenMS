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
#include "../Private/ChannelAddress.h"

class WSIPv4Address : public IPv4Address, public IWebSocketAddress
{
public:
	WSIPv4Address(MSStringView ip, uint16_t port, MSStringView host = MSStringView())
		:
		IPv4Address(ip, port, host)
	{
	}
};

class WSIPv6Address : public IPv6Address, public IWebSocketAddress
{
public:
	WSIPv6Address(MSStringView ip, uint16_t port, MSStringView host = MSStringView())
		:
		IPv6Address(ip, port, host)
	{
	}
};