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
#include "MS.h"
#include "IMailPromise.h"
class IMailContext;
using IMailTask = IMailPromise<void>;

/// @brief Interface for mail
struct IMail
{
	MSString From, To, Data;
	uint32_t FromSID, ToSID; // For Session
};

/// @brief Interface for mailbox
class OPENMS_API IMailBox
{
public:
	virtual ~IMailBox() = default;

	virtual bool send(IMail&& mail) = 0;

	// virtual bool send(IMail&& mail, IMail& response) = 0;

	virtual bool create(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory) = 0;

	virtual bool cancel(MSString address) = 0;

	virtual bool exist(MSString address) const = 0;

	template<class T, class... ARGS>
	bool create(MSString address, ARGS... args)
	{
		static_assert(std::is_base_of_v<IMailBox, T>);
		return create(address, [&](MSRaw<IMailContext> context){ return MSNew<T>(context, std::forward<ARGS>(args)...); });
	}

protected:
	virtual void error(MSError&& info) = 0;

	virtual IMailTask read(IMail&& mail) = 0;
};