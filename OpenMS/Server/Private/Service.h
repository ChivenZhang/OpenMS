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
#include "Utility/Timer.h"

class Service : public MailBox
{
public:
	using method_t = MSLambda<MSAsync<MSString>(MSStringView)>;
	using session_t = MSLambda<void(MSStringView)>;
	using MailBox::MailBox;
	bool bind(MSStringView method, method_t callback);
	bool call(MSStringView service, MSStringView method, uint32_t timeout, MSStringView request, MSString& response);
	bool async(MSStringView service, MSStringView method, uint32_t timeout, MSStringView request, MSLambda<void(MSStringView)> callback);

protected:
	IMailTask read(IMail mail) final;

protected:
	Timer m_Timer;
	MSMutex m_MutexMethod;
	MSMutex m_MutexSession;
	MSAtomic<uint32_t> m_SessionID;
	MSMap<uint32_t, method_t> m_MethodMap;
	MSMap<uint32_t, session_t> m_SessionMap;
};
