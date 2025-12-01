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
	explicit GuestService(MSHnd<IChannel> client);

protected:
	IMailTask read(IMail mail) override;

protected:
	MSHnd<IChannel> m_ClientChannel;
};