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

class ForwardService : public Service
{
public:
	explicit ForwardService(MSHnd<IChannel> client);

protected:
	IMailTask onRead(IMail mail) override;

protected:
	MSHnd<IChannel> m_ClientChannel;
};
