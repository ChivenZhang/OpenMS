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

/// @brief Interface for mail context
class OPENMS_API IMailContext
{
public:
	virtual ~IMailContext() = default;

	virtual bool createMailbox(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory) = 0;

	virtual bool cancelMailbox(MSString address) = 0;

	virtual bool existMailbox(MSString address) = 0;

	virtual bool sendToMailbox(IMail&& mail) = 0;

	virtual bool sendToMailbox(MSLambda<bool(IMail&& mail)> func) = 0;

	virtual void listMailbox(MSStringList& result) = 0;

	template<class T, class... Args>
	bool createMailbox(MSString address, Args... args)
	{
		static_assert(std::is_base_of_v<IMailBox, T>);
		return createMailbox(address, [&](MSRaw<IMailContext> context){ return MSNew<T>(context, std::forward<Args>(args)...); });
	}
};
using IMailContextRaw = MSRaw<IMailContext>;