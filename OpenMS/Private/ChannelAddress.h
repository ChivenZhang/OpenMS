#pragma once
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
#include "IChannelAddress.h"

class IPv4Address : public ISocketAddress
{
public:
	IPv4Address(TStringView ip, uint16_t port, TStringView host = TStringView());
	TString getAddress() const override;
	TString getHostName() const override;
	uint16_t getPort() const override;

protected:
	uint16_t m_PortNum;
	TString m_Address, m_HostName;
};

class IPv6Address : public ISocketAddress
{
public:
	IPv6Address(TStringView ip, uint16_t port, TStringView host = TStringView());
	TString getAddress() const override;
	TString getHostName() const override;
	uint16_t getPort() const override;

protected:
	uint16_t m_PortNum;
	TString m_Address, m_HostName;
};