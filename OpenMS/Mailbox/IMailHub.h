#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "IMailBox.h"

/// @brief Interface for mail hub
class OPENMS_API IMailHub
{
public:
	virtual ~IMailHub() = default;

	virtual bool create(MSString address, MSRef<IMailBox> value) = 0;

	virtual bool cancel(MSString address) = 0;

	virtual bool exist(MSString address) = 0;

	virtual uint32_t send(IMail mail) = 0;

	virtual bool send(MSLambda<bool(IMail mail)> func) = 0;

	virtual void list(MSList<uint32_t>& result) = 0;
};