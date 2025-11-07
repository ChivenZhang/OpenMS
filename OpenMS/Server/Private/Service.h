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
#include "Mailbox/Private/MailBox.h"

class Service : public MailBox
{
public:
	using MailBox::MailBox;



protected:
	IMailTask read(IMail mail) override;

protected:
	using async_t = MSLambda<MSAsync<MSString>(MSString const&)>;
	MSMap<uint32_t, async_t> m_AsyncMap;
};