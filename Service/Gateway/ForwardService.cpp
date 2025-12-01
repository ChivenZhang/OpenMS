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
#include "ForwardService.h"

ForwardService::ForwardService(MSHnd<IChannel> client)
	:
	m_ClientChannel(client)
{
}

IMailTask ForwardService::onRead(IMail mail)
{
	// TODO: 转发消息到客户端
	return Service::onRead(mail);
}
