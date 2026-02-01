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
#include "MS.h"
#include <iocpp.h>
#include "Utility/TimerUtility.h"

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

/// @brief Interface for server
class OPENMS_API IServer
{
public:
	using timer_t = TimerUtility::TimerId;

public:
	virtual ~IServer() = default;

	virtual int startup() = 0;

	virtual void shutdown() = 0;

	virtual MSString identity() const = 0;

	virtual void postEvent(MSLambda<void()>&& event) = 0;

	virtual MSString property(MSString const& name) const = 0;

	virtual timer_t startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void()>&& task) = 0;

	virtual void stopTimer(timer_t handle) = 0;

	template <class T>
	T property(MSString const& name, T const& value = T()) const
	{
		T result;
		if (MSTypeC(property(name), result)) return result;
		return value;
	}
};

#include "IStartup.h"
