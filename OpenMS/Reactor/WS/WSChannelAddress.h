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
	static MSRef<WSIPv4Address> New(MSStringView ip, uint16_t port, MSStringView path = MSStringView(), MSStringView host = MSStringView());

public:
	WSIPv4Address(MSStringView ip, uint16_t port, MSStringView path = MSStringView(), MSStringView host = MSStringView());
	MSString getString() const override;
	MSString getPath() const override;

protected:
	MSString m_Path;
};

class WSIPv6Address : public IPv6Address, public IWebSocketAddress
{
public:
	static MSRef<WSIPv6Address> New(MSStringView ip, uint16_t port, MSStringView path = MSStringView(), MSStringView host = MSStringView());

public:
	WSIPv6Address(MSStringView ip, uint16_t port, MSStringView path = MSStringView(), MSStringView host = MSStringView());
	MSString getString() const override;
	MSString getPath() const override;

protected:
	MSString m_Path;
};