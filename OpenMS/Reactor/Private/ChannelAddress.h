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
#include "../IChannelAddress.h"

class IPv4Address : public ISocketAddress
{
public:
	static MSRef<IPv4Address> New(MSStringView ip, uint16_t port, MSStringView host = MSStringView());

public:
	IPv4Address(MSStringView ip, uint16_t port, MSStringView host = MSStringView());
	MSString getAddress() const override;
	MSString getHostName() const override;
	uint32_t getHashName() const override;
	MSString getString() const override;
	uint16_t getPort() const override;

protected:
	uint16_t m_PortNum;
	MSString m_Address, m_HostName;
};

class IPv6Address : public ISocketAddress
{
public:
	static MSRef<IPv6Address> New(MSStringView ip, uint16_t port, MSStringView host = MSStringView());

public:
	IPv6Address(MSStringView ip, uint16_t port, MSStringView host = MSStringView());
	MSString getAddress() const override;
	MSString getHostName() const override;
	uint32_t getHashName() const override;
	MSString getString() const override;
	uint16_t getPort() const override;

protected:
	uint16_t m_PortNum;
	MSString m_Address, m_HostName;
};