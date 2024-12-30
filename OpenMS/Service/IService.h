#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "MS.h"
#include <iocpp.h>

#define OPENMS_LOGO \
(R"(
  ___                   __  __ ____  
 / _ \ _ __   ___ _ __ |  \/  / ___| 
| | | | '_ \ / _ \ '_ \| |\/| \___ \ 
| |_| | |_) |  __/ | | | |  | |___) |
 \___/| .__/ \___|_| |_|_|  |_|____/ 
======|_|============================

:: OpenMS ::                (v1.0.0)

)")

/// @brief Interface for service
class OPENMS_API IService
{
public:
	virtual ~IService() = default;

	virtual int startup() = 0;

	virtual void shutdown() = 0;

	virtual MSString identity() const = 0;

	virtual void sendEvent(MSLambda<void()>&& event) = 0;

	virtual uint32_t startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void(uint32_t handle)>&& task) = 0;

	virtual bool stopTimer(uint32_t handle) = 0;

	virtual MSString property(MSString const& name) const = 0;

	template <class T>
	T property(MSString const& name, T const& value = T()) const
	{
		T result;
		if (TTypeC(property(name), result)) return result;
		return value;
	}
};

#include "IBootstrap.h"
