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

class GuestService : public Service
{
public:
	explicit GuestService(MSHnd<IChannel> client, uint32_t guestID);

protected:
	IMailTask read(IMail mail) override;

protected:
	uint32_t m_GuestID;
	MSHnd<IChannel> m_ClientChannel;
};