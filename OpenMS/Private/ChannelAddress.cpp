/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "ChannelAddress.h"

IPv4Address::IPv4Address(TStringView ip, uint16_t port, TStringView host)
	:
	m_Address(ip),
	m_PortNum(port),
	m_HostName(host)
{
}

TString IPv4Address::getAddress() const
{
	return m_Address;
}

TString IPv4Address::getHostName() const
{
	return m_HostName;
}

uint16_t IPv4Address::getPort() const
{
	return m_PortNum;
}

IPv6Address::IPv6Address(TStringView ip, uint16_t port, TStringView host)
	:
	m_Address(ip),
	m_PortNum(port),
	m_HostName(host)
{
}

TString IPv6Address::getAddress() const
{
	return m_Address;
}

TString IPv6Address::getHostName() const
{
	return m_HostName;
}

uint16_t IPv6Address::getPort() const
{
	return m_PortNum;
}