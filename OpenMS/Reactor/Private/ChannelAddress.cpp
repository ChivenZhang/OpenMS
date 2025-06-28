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
#include "ChannelAddress.h"

MSRef<IPv4Address> IPv4Address::New(MSStringView ip, uint16_t port, MSStringView host)
{
	return MSNew<IPv4Address>(ip, port, host);
}

IPv4Address::IPv4Address(MSStringView ip, uint16_t port, MSStringView host)
	:
	m_Address(ip),
	m_PortNum(port),
	m_HostName(host)
{
}

MSString IPv4Address::getAddress() const
{
	return m_Address;
}

MSString IPv4Address::getHostName() const
{
	return m_HostName;
}

uint32_t IPv4Address::getHashName() const
{
	return MSHash(getString());
}

MSString IPv4Address::getString() const
{
	return m_Address + ":" + std::to_string(m_PortNum);
}

uint16_t IPv4Address::getPort() const
{
	return m_PortNum;
}

MSRef<IPv6Address> IPv6Address::New(MSStringView ip, uint16_t port, MSStringView host)
{
	return MSNew<IPv6Address>(ip, port, host);
}

IPv6Address::IPv6Address(MSStringView ip, uint16_t port, MSStringView host)
	:
	m_Address(ip),
	m_PortNum(port),
	m_HostName(host)
{
}

MSString IPv6Address::getAddress() const
{
	return m_Address;
}

MSString IPv6Address::getHostName() const
{
	return m_HostName;
}

uint32_t IPv6Address::getHashName() const
{
	return MSHash(getString());
}

MSString IPv6Address::getString() const
{
	return m_Address + ":" + std::to_string(m_PortNum);
}

uint16_t IPv6Address::getPort() const
{
	return m_PortNum;
}