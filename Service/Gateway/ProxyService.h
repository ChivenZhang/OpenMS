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
#include <Server/Private/Service.h>

class ProxyService : public Service
{
public:
	explicit ProxyService(MSHnd<IChannel> client, uint32_t userID);

protected:
	IMailTask read(IMail mail) override;

protected:
	uint32_t m_UserID;
	MSHnd<IChannel> m_ClientChannel;
};
