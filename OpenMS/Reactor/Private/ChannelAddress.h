#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "../IChannelAddress.h"

class IPv4Address : public ISocketAddress
{
public:
	static TRef<IPv4Address> New(TStringView ip, uint16_t port, TStringView host = TStringView());

public:
	IPv4Address(TStringView ip, uint16_t port, TStringView host = TStringView());
	TString getAddress() const override;
	TString getHostName() const override;
	uint32_t getHashName() const override;
	TString getString() const override;
	uint16_t getPort() const override;

protected:
	uint16_t m_PortNum;
	TString m_Address, m_HostName;
};

class IPv6Address : public ISocketAddress
{
public:
	static TRef<IPv6Address> New(TStringView ip, uint16_t port, TStringView host = TStringView());

public:
	IPv6Address(TStringView ip, uint16_t port, TStringView host = TStringView());
	TString getAddress() const override;
	TString getHostName() const override;
	uint32_t getHashName() const override;
	TString getString() const override;
	uint16_t getPort() const override;

protected:
	uint16_t m_PortNum;
	TString m_Address, m_HostName;
};