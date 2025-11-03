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

	virtual bool create(MSString address, MSLambda<MSRef<IMailBox>()> factory) = 0;

	virtual bool cancel(MSString address) = 0;

	virtual bool exist(MSString address) = 0;

	virtual uint32_t send(IMail mail) = 0;

	virtual bool send(MSLambda<bool(IMail mail)> func) = 0;

	virtual void list(MSStringList& result) = 0;

	template<class T, class... Args>
	bool create(MSString address, Args &&... args)
	{
		static_assert(std::is_base_of_v<IMailBox, T>);
		return create(address, [&](){ return MSNew<T>(this, std::forward<Args>(args)...); });
	}
};
using IMailHubRaw = MSRaw<IMailHub>;