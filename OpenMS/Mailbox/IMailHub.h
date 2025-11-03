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

	virtual bool createMailbox(MSString address, MSLambda<MSRef<IMailBox>()> factory) = 0;

	virtual bool cancelMailbox(MSString address) = 0;

	virtual bool existMailbox(MSString address) = 0;

	virtual uint32_t sendToMailbox(IMail mail) = 0;

	virtual bool sendToMailbox(MSLambda<bool(IMail mail)> func) = 0;

	virtual void listMailbox(MSStringList& result) = 0;

	template<class T, class... Args>
	bool createMailbox(MSString address, Args &&... args)
	{
		static_assert(std::is_base_of_v<IMailBox, T>);
		return createMailbox(address, [&](){ return MSNew<T>(this, std::forward<Args>(args)...); });
	}
};
using IMailHubRaw = MSRaw<IMailHub>;