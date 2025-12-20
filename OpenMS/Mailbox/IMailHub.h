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

	/// create mailbox
	/// @param address
	/// @param value
	/// @return
	virtual bool create(MSString address, MSRef<IMailBox> value) = 0;

	/// delete mailbox
	/// @param address
	/// @return
	virtual bool cancel(MSString address) = 0;

	/// check mailbox exist
	/// @param address
	/// @return
	virtual bool exist(MSString address) = 0;

	/// send mail
	/// @param mail
	/// @return
	virtual uint32_t send(IMail mail) = 0;

	/// list mailboxes
	/// @param result
	virtual void list(MSList<uint32_t>& result) = 0;

	/// call on mail send failed
	/// @param callback
	/// @return
	virtual bool failed(MSLambda<bool(IMail mail)> callback) = 0;

	/// call on mailbox change
	/// @param callback
	/// @return
	virtual bool change(MSLambda<void(MSString address)> callback) = 0;
};