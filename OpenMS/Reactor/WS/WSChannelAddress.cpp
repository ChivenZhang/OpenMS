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
#include "WSChannelAddress.h"

MSRef<WSIPv4Address> WSIPv4Address::New(MSStringView ip, uint16_t port, MSStringView path, MSStringView host)
{
	return MSNew<WSIPv4Address>(ip, port, path, host);
}

WSIPv4Address::WSIPv4Address(MSStringView ip, uint16_t port, MSStringView path, MSStringView host)
	:
	IPv4Address(ip, port, host),
	m_Path(path)
{
}

MSString WSIPv4Address::getString() const
{
	return m_Address + ":" + std::to_string(m_PortNum) + m_Path;
}

MSString WSIPv4Address::getPath() const
{
	return m_Path;
}

MSRef<WSIPv6Address> WSIPv6Address::New(MSStringView ip, uint16_t port, MSStringView path, MSStringView host)
{
	return MSNew<WSIPv6Address>(ip, port, path, host);
}

WSIPv6Address::WSIPv6Address(MSStringView ip, uint16_t port, MSStringView path, MSStringView host)
	:
	IPv6Address(ip, port, host),
	m_Path(path)
{
}

MSString WSIPv6Address::getString() const
{
	return m_Address + ":" + std::to_string(m_PortNum) + "/" + m_Path;
}

MSString WSIPv6Address::getPath() const
{
	return m_Path;
}
