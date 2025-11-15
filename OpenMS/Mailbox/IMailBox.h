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
#include "IMail.h"
#include "Utility/Async.h"
class IMailHub;
using IMailTask = MSAsync<void>;

/// @brief Interface for mailbox
class OPENMS_API IMailBox
{
public:
	using name_t = uint32_t;

public:
	virtual ~IMailBox() = default;

	virtual name_t name() const = 0;

	virtual uint32_t send(IMail mail) = 0;

	virtual bool create(MSString address, MSRef<IMailBox> value) = 0;

	virtual bool cancel(MSString address) = 0;

	virtual bool exist(MSString address) const = 0;

	template<class T, class... ARGS>
	bool create(MSString address, ARGS &&... args)
	{
		static_assert(std::is_base_of_v<IMailBox, T>);
		return create(address, [&](){ return MSNew<T>(this, std::forward<ARGS>(args)...); });
	}
};